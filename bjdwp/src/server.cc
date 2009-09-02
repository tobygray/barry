#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <algorithm>
#include <functional>
#include <vector>

#include <barry/endian.h>
#include <barry/barry.h>

#include "message.h"
#include "handler.h"
#include "server.h"
#include "protocol.h"
#include "data.h"


using namespace std;
using namespace Barry;
using namespace JDG;


namespace JDWP {

static void * acceptWrapper(void *data);


JDWServer::JDWServer(const char *address, int port)
{
	sockfd = -1;
	handler = NULL;

	this->address = strdup(address);
	this->port = port;
	this->printConsoleMessage = NULL;

	searchDebugFile(debugFileList);
}


JDWServer::~JDWServer() 
{
	stop();

	free(address);
}


void JDWServer::setDevice(Barry::Mode::JVMDebug *device) 
{
	jvmdebug = device;
}


void JDWServer::setPasswordDevice(string password)
{
	this->password = password;
}


void JDWServer::setConsoleCallback(void (*callback)(string message))
{
	printConsoleMessage = callback;
}


bool JDWServer::start() 
{
	int rc;

	struct hostent *hp;
	struct sockaddr_in sad;


	memset((char *) &sad, '\0', sizeof(struct sockaddr_in));

	if (address == NULL)
		sad.sin_addr.s_addr = INADDR_ANY;
	else {
		sad.sin_addr.s_addr = inet_addr(address);

		if (sad.sin_addr.s_addr == INADDR_NONE) {
			hp = gethostbyname(address);

			if (hp == NULL) {
				// TODO 
				// throw Exception...
				exit(-1);
			}

			memcpy((char*) &sad.sin_addr, (char*) hp->h_addr, (size_t) hp->h_length);
		}
	}

	sad.sin_family = AF_INET;
	sad.sin_port = htons((short) (port & 0xFFFF));

	// Open socket
	sockfd = socket(sad.sin_family, SOCK_STREAM, 0);

	if (sockfd < 0) {
		// TODO
		// Throw Exception
		exit(-1);
	}

	// Bind
	rc = bind(sockfd, (struct sockaddr *) &sad, sizeof(sad));

	if (rc < 0) {
		close(sockfd);
		sockfd = -1;

		// TODO 
		// Throw Exception
		exit(-1);
	}

	// Listen
	if (listen(sockfd, SOMAXCONN) < 0) {
		close(sockfd);
		sockfd = -1;

		// TODO
		// Throw Exception
		exit(-1);
	}

	handler = new JDWHandler(sockfd, acceptWrapper, (void*) this);

	return true;
}


static void * acceptWrapper(void *data) {
	JDWServer *s;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	s = (JDWServer *) data;

	while (1) {
		s->acceptConnection();

		s->attachToDevice();

		s->initVisibleClassList();
		
		if (s->hello()) {
			s->run();

			s->detachFromDevice();
		}
	}

	return NULL;
}


void JDWServer::acceptConnection() {
	size_t addrlen;

	struct sockaddr *sa;
	struct sockaddr_in addr;

	sa = (struct sockaddr*) &addr;

	addrlen = sizeof(addr);

	fd = accept(sockfd, sa, &addrlen);

	fcntl(fd, F_SETFL, O_NONBLOCK);

	if (fd < 0)
		return;
}


void JDWServer::attachToDevice() {
	bool ret;

	targetrunning = false;

	jvmdebug->Open(password.c_str());
	jvmdebug->Attach();

	jvmdebug->Unknown01();
	jvmdebug->Unknown02();
	jvmdebug->Unknown03();
	jvmdebug->Unknown04();
	jvmdebug->Unknown05();

	jvmdebug->GetModulesList(modulesList);
	cout << modulesList;

	// Check debug info for each modules
	vector<JVMModulesEntry>::iterator b = modulesList.begin();
	for ( ; b != modulesList.end(); b++) {
		JDGCodInfo codInfo;

		JVMModulesEntry entry = *b;
		
		ret = loadDebugInfo(debugFileList, entry.UniqueID, entry.Name, codInfo);

		if (ret == true) {
			appList[entry.UniqueID].load(codInfo);
		}
		else {
			cout << "No debug information found for '" << entry.Name;
			cout << "' (" << hex << setfill('0') << setw(8) << entry.UniqueID << ")." << endl;
		}
	}
}


void JDWServer::detachFromDevice() {
	jvmdebug->Detach();
	jvmdebug->Close();
}


#define JDWP_HELLO_STRING		"JDWP-Handshake"



bool JDWServer::hello() {
	bool ret;

	Barry::Data response;
	
	const size_t len = strlen(JDWP_HELLO_STRING);

	JDWMessage msg(fd);

	do {
		ret = msg.Receive(response);
	}
	while (!ret);

	int size = response.GetSize();
	char *str = (char *) response.GetBuffer();

	if (size != len)
		return false;
	
	if (!strncmp(str, JDWP_HELLO_STRING, len)) {
		Data command(JDWP_HELLO_STRING, len);

		msg.Send(command);

		return true;
	}

	return false;
}


void JDWServer::run() {
	string str;
	JDWMessage msg(fd);

	Barry::Data command;

	MAKE_JDWPPACKET(rpack, command);

	loop = true;

	while (loop) {
		if (targetrunning) {
			// Read JDWP message from device
			int value = jvmdebug->GetConsoleMessage(str);

			if (value < 0) {
				bool ret;
				int status;

				ret = jvmdebug->GetStatus(status);

				while (!ret) {
					// Read JDB message from host
					msg.Receive(command);
			
					if (command.GetSize() > 0) {
						// Convert to packet
						rpack = (const Protocol::Packet *) command.GetData();

						if (command.GetSize() != be_btohl(rpack->length)) {
							cout << "Packet size error !!!" << endl;

							// TODO : add throw exception

							continue;
						}

						CommandsetProcess(command);
	
						break;
					}
					else
						ret = jvmdebug->WaitStatus(status);
				}
			}
			else {
				if (printConsoleMessage != NULL)
					printConsoleMessage(str);
			}
		}
		else {
			// Read JDB message from host
			msg.Receive(command);

			if (command.GetSize() > 0) {
				// Convert to packet
				rpack = (const Protocol::Packet *) command.GetData();

				if (command.GetSize() != be_btohl(rpack->length)) {
					cout << "Packet size error !!!" << endl;

					// TODO : add throw exception

					continue;
				}

				CommandsetProcess(command);
			}

			usleep(50);
		}
	}
}


bool JDWServer::stop() {
	if (handler) {
		handler->dispose();
		handler = 0;
	}

	if (sockfd >= 0)
		close(sockfd);

	sockfd = -1;

	return true;
}


void JDWServer::initVisibleClassList() {
	int index;

	// Skip the cell '0'
	// it's very ugly, but I want use an index started at '1' inside of '0'
	// JDB works from '1' :(
	JDGClassEntry e;
	visibleClassList.push_back(e);

	// Count and index the class (start to '1')
	index = 1;
	map<uint32_t, JDWAppInfo>::iterator it;

	for (it = appList.begin(); it != appList.end(); it++) {
		JDWAppInfo *appInfo = &(it->second);
		JDGClassList *list = &(appInfo->classList);
	
		vector<JDGClassEntry>::iterator b;

		for (b = list->begin(); b != list->end(); b++) {
			// FIXME
			// I don't from class field, we have to filter the class view by JDB
//			if ((b->type != 0x824) && (b->type != 0x64)) {
			if (b->id == 0xffffffff) {
				b->index = -1;

				continue;
			}

			b->index = index;

			visibleClassList.push_back(*b);

			index++;
		}
	}

	visibleClassList.createDefaultEntries();
}


void JDWServer::CommandsetProcess(Data &cmd) {
	MAKE_JDWPPACKET(rpack, cmd);

	switch (rpack->u.command.commandset) {
		case JDWP_CMDSET_VIRTUALMACHINE:
			CommandsetVirtualMachineProcess(cmd);
			break;

		case JDWP_CMDSET_REFERECENTYPE:
			break;

		case JDWP_CMDSET_CLASSTYPE:
			break;

		case JDWP_CMDSET_ARRAYTYPE:
			break;

		case JDWP_CMDSET_INTERFACETYPE:
			break;

		case JDWP_CMDSET_METHOD:
			break;

		case JDWP_CMDSET_FIELD:
			break;

		case JDWP_CMDSET_OBJECTREFERENCE:
			break;

		case JDWP_CMDSET_STRINGREFERENCE:
			break;

		case JDWP_CMDSET_THREADREFERENCE:
			break;

		case JDWP_CMDSET_THREADGROUPREFERENCE:
			break;

		case JDWP_CMDSET_ARRAYREFERENCE:
			break;

		case JDWP_CMDSET_CLASSLOADERREFERENCE:
			break;

		case JDWP_CMDSET_EVENTREQUEST:
			CommandsetEventRequestProcess(cmd);
			break;

		case JDWP_CMDSET_STACKFRAME:
			break;

		case JDWP_CMDSET_CLASSOBJECTREFERENCE:
			break;

		case JDWP_CMDSET_EVENT:
			break;

		default:
			// TODO : add exception (or alert)
			cout << "Commandset unknown !!!" << endl;
	}
}


void JDWServer::CommandsetVirtualMachineProcess(Data &cmd) {
	JDWMessage msg(fd);

	MAKE_JDWPPACKET(rpack, cmd);

	switch (rpack->u.command.command) {
		case JDWP_CMD_VERSION:
			CommandVersion(cmd);
			break;

		case JDWP_CMD_ALLCLASSES:
			CommandAllClasses(cmd);
			break;

		case JDWP_CMD_ALLTHREADS:
			CommandAllThreads(cmd);
			break;

		case JDWP_CMD_DISPOSE:
			loop = false;
			targetrunning = false;
			close(fd);
			break;

		case JDWP_CMD_IDSIZES:
			CommandIdSizes(cmd);
			break;

		case JDWP_CMD_SUSPEND:
			CommandSuspend(cmd);
			targetrunning = false;
			break;

		case JDWP_CMD_RESUME:
			CommandResume(cmd);
			targetrunning = true;
			break;

		case JDWP_CMD_CLASSPATHS:
			CommandClassPaths(cmd);
			break;
	}
}


void JDWServer::CommandsetEventRequestProcess(Data &cmd) {
	JDWMessage msg(fd);

	MAKE_JDWPPACKET(rpack, cmd);

	switch (rpack->u.command.command) {
		case JDWP_CMD_SET:
			CommandSet(cmd);
			break;
	}
}


void JDWServer::CommandVersion(Data &cmd) {
	JDWMessage msg(fd);

	// Build packet data
	Data response;

	size_t offset = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	AddDataString(response, offset, string("RIM JVM"));
	AddDataInt(response, offset, be_htobl(1));
	AddDataInt(response, offset, be_htobl(4));
	AddDataString(response, offset, string("1.4"));
	AddDataString(response, offset, string("RIM JVM"));

	response.ReleaseBuffer(offset);


	size_t total_size = response.GetSize();

	// Fill in the header values
	MAKE_JDWPPACKETPTR_BUF(cpack, response.GetBuffer(total_size));
	Protocol::Packet &packet = *cpack;


	MAKE_JDWPPACKET(rpack, cmd);

	packet.length = be_htobl(total_size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	response.ReleaseBuffer(total_size);
	msg.Send(response);
}


void JDWServer::CommandAllClasses(Data &cmd) {
	int i;
	int size;

	JDWMessage msg(fd);

	// Build packet data
	Data response;

	size_t offset = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	// Size of known class list
	size = visibleClassList.size() - 1;

	AddDataInt(response, offset, be_htobl(size));

	// Then, write the list of known class
	for (i=1; i<visibleClassList.size(); i++) {
		string str = visibleClassList[i].getFullClassName();

		str = "L" + str + ";";
		replace_if(str.begin(), str.end(), bind2nd(equal_to<char>(),'.'), '/');

		AddDataByte(response, offset, 0x01);
		AddDataInt(response, offset, i);	// Should be equal to visibleClassList[i].index
		AddDataString(response, offset, str);
		AddDataInt(response, offset, be_htobl(0x04));
	}

	response.ReleaseBuffer(offset);


	size_t total_size = response.GetSize();

	// Fill in the header values
	MAKE_JDWPPACKETPTR_BUF(cpack, response.GetBuffer(total_size));
	Protocol::Packet &packet = *cpack;


	MAKE_JDWPPACKET(rpack, cmd);

	packet.length = be_htobl(total_size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	response.ReleaseBuffer(total_size);
	msg.Send(response);
}


void JDWServer::CommandAllThreads(Data &cmd) {
	JDWMessage msg(fd);

	// Get threads list from device
	JVMThreadsList list;
	jvmdebug->GetThreadsList(list);
	cout << list;

	// Build packet data
	Data response;

	size_t offset = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	// Indicate the number of element
	AddDataInt(response, offset, be_htobl(list.size()));

	// Send all threads ID
	vector<JVMThreadsEntry>::iterator b = list.begin();
	for( ; b != list.end(); b++ ) {
		JVMThreadsEntry entry = *b;

		AddDataInt(response, offset, be_htobl(entry.Id));
	}

	response.ReleaseBuffer(offset);


	size_t total_size = response.GetSize();

	// Fill in the header values
	MAKE_JDWPPACKETPTR_BUF(cpack, response.GetBuffer(total_size));
	Protocol::Packet &packet = *cpack;


	MAKE_JDWPPACKET(rpack, cmd);

	packet.length = be_htobl(total_size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	response.ReleaseBuffer(total_size);
	msg.Send(response);
}


void JDWServer::CommandIdSizes(Data &cmd) {
	JDWMessage msg(fd);

	MAKE_JDWPPACKET(rpack, cmd);

	size_t size;

	Protocol::Packet packet;

	size = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE
		+ JDWP_PACKETVIRTUALMACHINEIDSIZES_DATA_SIZE;

	packet.length = be_htobl(size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);
	packet.u.response.u.virtualMachine.u.IDSizes.fieldIDSize = be_htobl(0x04);
	packet.u.response.u.virtualMachine.u.IDSizes.methodIDSize = be_htobl(0x04);
	packet.u.response.u.virtualMachine.u.IDSizes.objectIDSize = be_htobl(0x04);
	packet.u.response.u.virtualMachine.u.IDSizes.referenceTypeIDSize = be_htobl(0x04);
	packet.u.response.u.virtualMachine.u.IDSizes.frameIDSize = be_htobl(0x04);

	Data response(&packet, size);

	msg.Send(response);
}


void JDWServer::CommandSuspend(Data &cmd) {
	JDWMessage msg(fd);


	// Suspend device
	jvmdebug->Stop();

	// Notify debugger
	MAKE_JDWPPACKET(rpack, cmd);

	size_t size;

	Protocol::Packet packet;

	size = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	packet.length = be_htobl(size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	Data response(&packet, size);

	msg.Send(response);
}


void JDWServer::CommandResume(Data &cmd) {
	JDWMessage msg(fd);


	// Resume device
	jvmdebug->Unknown06();
	jvmdebug->Unknown07();
	jvmdebug->Unknown08();
	jvmdebug->Unknown09();
	jvmdebug->Unknown10();
	jvmdebug->Go();

	// Notify debugger
	MAKE_JDWPPACKET(rpack, cmd);

	size_t size;

	Protocol::Packet packet;

	size = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	packet.length = be_htobl(size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	Data response(&packet, size);

	msg.Send(response);
}


void JDWServer::CommandClassPaths(Data &cmd) {
	JDWMessage msg(fd);

	// Build packet data
	Data response;

	size_t offset = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE;

	AddDataString(response, offset, string(""));
	AddDataInt(response, offset, be_htobl(0));
	AddDataInt(response, offset, be_htobl(0));

	response.ReleaseBuffer(offset);


	size_t total_size = response.GetSize();

	// Fill in the header values
	MAKE_JDWPPACKETPTR_BUF(cpack, response.GetBuffer(total_size));
	Protocol::Packet &packet = *cpack;


	MAKE_JDWPPACKET(rpack, cmd);

	packet.length = be_htobl(total_size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);

	response.ReleaseBuffer(total_size);
	msg.Send(response);
}



void JDWServer::CommandSet(Data &cmd) {
	static int value = 2;

	JDWMessage msg(fd);

	MAKE_JDWPPACKET(rpack, cmd);

	size_t size;

	Protocol::Packet packet;

	size = JDWP_PACKET_HEADER_SIZE + JDWP_RESPONSE_HEADER_SIZE + sizeof(uint32_t);

	packet.length = be_htobl(size);
	packet.id = rpack->id;
	packet.flags = 0x80;
	packet.u.response.errorcode = be_htobs(0);
	packet.u.response.u.value = be_htobl(value);

	Data response(&packet, size);

	msg.Send(response);

	value++;
}


} // namespace JDWP

