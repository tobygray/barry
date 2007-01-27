// Generated by gtkmmproc -- DO NOT MODIFY!


//
// This was copied from gtkmm 2.10.6 and hacked up a bit, so
// that we have a dialog class for systems using older
// versions of gtkmm24.  Debian stable, I'm looking at you...
//
// Chris Frey <cdfrey@foursquare.net>
//

#include "aboutdialog.h"

// -*- c++ -*-
/* $Id$ */

/* 
 *
 * Copyright 2004 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtk/gtkaboutdialog.h>
#include <gtk/gtklabel.h>

//namespace Gtk {  class AboutDialog_Class { public: static Glib::ObjectBase* wrap_new(GObject*); };  }

//  Glib::wrap_register(gtk_about_dialog_get_type(), &Gtk::AboutDialog_Class::wrap_new);


#include <gtkmm/private/dialog_p.h>

#include <glibmm/class.h>

namespace Gtk
{

class AboutDialog_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef AboutDialog CppObjectType;
  typedef GtkAboutDialog BaseObjectType;
  typedef GtkAboutDialogClass BaseClassType;
  typedef Gtk::Dialog_Class CppClassParent;
  typedef GtkDialogClass BaseClassParent;

  friend class AboutDialog;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();

  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  //Callbacks (virtual functions):
#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED
};


} // namespace Gtk



//We use a function instead of a static method, so we can make it static, so it is not exported.
static void SignalProxy_ActivateLink_gtk_callback(GtkAboutDialog* about, const gchar* link, gpointer data)
{
  Gtk::AboutDialog::SlotActivateLink* the_slot = static_cast<Gtk::AboutDialog::SlotActivateLink*>(data);

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif //GLIBMM_EXCEPTIONS_ENABLED
    Gtk::AboutDialog* pCppAbout = Glib::wrap(about);
    (*the_slot)( *pCppAbout, Glib::convert_const_gchar_ptr_to_ustring(link) );
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  }
  catch(...)
  {
    Glib::exception_handlers_invoke();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED
}

static void SignalProxy_ActivateLink_gtk_callback_destroy(void* data)
{
  Gtk::AboutDialog::SlotActivateLink* the_slot = static_cast<Gtk::AboutDialog::SlotActivateLink*>(data);
  delete the_slot;
}


namespace Gtk
{

//static:
void AboutDialog::set_email_hook(const SlotActivateLink& slot)
{
  // Create a copy of the slot object. A pointer to this will be passed
  // through the callback's data parameter.  It will be deleted
  // when SignalProxy_SlotForwardPage_gtk_callback_destroy() is called.
  SlotActivateLink* slot_copy = new SlotActivateLink(slot);

  gtk_about_dialog_set_email_hook(
      &SignalProxy_ActivateLink_gtk_callback, slot_copy,
      &SignalProxy_ActivateLink_gtk_callback_destroy);
}
  
//static:
void AboutDialog::set_url_hook(const SlotActivateLink& slot)
{
  // Create a copy of the slot object. A pointer to this will be passed
  // through the callback's data parameter.  It will be deleted
  // when SignalProxy_SlotForwardPage_gtk_callback_destroy() is called.
  SlotActivateLink* slot_copy = new SlotActivateLink(slot);

  gtk_about_dialog_set_url_hook(
      &SignalProxy_ActivateLink_gtk_callback, slot_copy,
      &SignalProxy_ActivateLink_gtk_callback_destroy);
} 

} // namespace Gtk


namespace
{
} // anonymous namespace


namespace Glib
{

Gtk::AboutDialog* wrap(GtkAboutDialog* object, bool take_copy)
{
  return dynamic_cast<Gtk::AboutDialog *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& AboutDialog_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &AboutDialog_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_about_dialog_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void AboutDialog_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
}

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED


Glib::ObjectBase* AboutDialog_Class::wrap_new(GObject* o)
{
  return new AboutDialog((GtkAboutDialog*)(o)); //top-level windows can not be manage()ed.

}


/* The implementation: */

AboutDialog::AboutDialog(const Glib::ConstructParams& construct_params)
:
  Gtk::Dialog(construct_params)
{
  }

AboutDialog::AboutDialog(GtkAboutDialog* castitem)
:
  Gtk::Dialog((GtkDialog*)(castitem))
{
  }

AboutDialog::~AboutDialog()
{
  destroy_();
}

AboutDialog::CppClassType AboutDialog::aboutdialog_class_; // initialize static member

GType AboutDialog::get_type()
{
  return aboutdialog_class_.init().get_type();
}

GType AboutDialog::get_base_type()
{
  return gtk_about_dialog_get_type();
}


AboutDialog::AboutDialog()
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Dialog(Glib::ConstructParams(aboutdialog_class_.init()))
{
  }

Glib::ustring AboutDialog::get_name() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_name(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_name(const Glib::ustring& name)
{
gtk_about_dialog_set_name(gobj(), name.c_str()); 
}

Glib::ustring AboutDialog::get_version() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_version(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_version(const Glib::ustring& version)
{
gtk_about_dialog_set_version(gobj(), version.c_str()); 
}

Glib::ustring AboutDialog::get_copyright() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_copyright(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_copyright(const Glib::ustring& copyright)
{
gtk_about_dialog_set_copyright(gobj(), copyright.c_str()); 
}

Glib::ustring AboutDialog::get_comments() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_comments(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_comments(const Glib::ustring& comments)
{
gtk_about_dialog_set_comments(gobj(), comments.c_str()); 
}

Glib::ustring AboutDialog::get_license() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_license(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_license(const Glib::ustring& license)
{
gtk_about_dialog_set_license(gobj(), license.c_str()); 
}

Glib::ustring AboutDialog::get_website() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_website(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_website(const Glib::ustring& website)
{
gtk_about_dialog_set_website(gobj(), website.c_str()); 
}

Glib::ustring AboutDialog::get_website_label() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_website_label(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_website_label(const Glib::ustring& website_label)
{
gtk_about_dialog_set_website_label(gobj(), website_label.c_str()); 
}

Glib::StringArrayHandle AboutDialog::get_authors() const
{
  return Glib::StringArrayHandle(gtk_about_dialog_get_authors(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_authors(const Glib::StringArrayHandle& authors) const
{
gtk_about_dialog_set_authors(const_cast<GtkAboutDialog*>(gobj()), const_cast<const gchar**>((authors).data())); 
}

Glib::StringArrayHandle AboutDialog::get_documenters() const
{
  return Glib::StringArrayHandle(gtk_about_dialog_get_documenters(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_documenters(const Glib::StringArrayHandle& documenters)
{
gtk_about_dialog_set_documenters(gobj(), const_cast<const gchar**>((documenters).data())); 
}

Glib::StringArrayHandle AboutDialog::get_artists() const
{
  return Glib::StringArrayHandle(gtk_about_dialog_get_artists(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_artists(const Glib::StringArrayHandle& artists)
{
gtk_about_dialog_set_artists(gobj(), const_cast<const gchar**>((artists).data())); 
}

Glib::ustring AboutDialog::get_translator_credits() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_translator_credits(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_translator_credits(const Glib::ustring& translator_credits)
{
gtk_about_dialog_set_translator_credits(gobj(), translator_credits.c_str()); 
}

Glib::RefPtr<Gdk::Pixbuf> AboutDialog::get_logo()
{
  return Glib::wrap(gtk_about_dialog_get_logo(gobj()));
}

Glib::RefPtr<const Gdk::Pixbuf> AboutDialog::get_logo() const
{
  return const_cast<AboutDialog*>(this)->get_logo();
}

void AboutDialog::set_logo(const Glib::RefPtr<Gdk::Pixbuf>& logo)
{
gtk_about_dialog_set_logo(gobj(), Glib::unwrap(logo)); 
}

Glib::ustring AboutDialog::get_logo_icon_name() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_about_dialog_get_logo_icon_name(const_cast<GtkAboutDialog*>(gobj())));
}

void AboutDialog::set_logo_icon_name(const Glib::ustring& icon_name)
{
gtk_about_dialog_set_logo_icon_name(gobj(), icon_name.c_str()); 
}

/*
bool AboutDialog::get_wrap_license() const
{
  return gtk_about_dialog_get_wrap_license(const_cast<GtkAboutDialog*>(gobj()));
}

void AboutDialog::set_wrap_license(bool wrap_license)
{
gtk_about_dialog_set_wrap_license(gobj(), static_cast<int>(wrap_license)); 
}
*/


#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_name() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "name");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_name() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "name");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_version() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "version");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_version() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "version");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_copyright() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "copyright");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_copyright() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "copyright");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_comments() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "comments");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_comments() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "comments");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_website() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "website");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_website() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "website");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_website_label() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "website-label");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_website_label() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "website-label");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_license() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "license");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_license() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "license");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::StringArrayHandle> AboutDialog::property_authors() 
{
  return Glib::PropertyProxy<Glib::StringArrayHandle>(this, "authors");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle> AboutDialog::property_authors() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle>(this, "authors");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::StringArrayHandle> AboutDialog::property_documenters() 
{
  return Glib::PropertyProxy<Glib::StringArrayHandle>(this, "documenters");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle> AboutDialog::property_documenters() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle>(this, "documenters");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::StringArrayHandle> AboutDialog::property_translator_credits() 
{
  return Glib::PropertyProxy<Glib::StringArrayHandle>(this, "translator-credits");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle> AboutDialog::property_translator_credits() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle>(this, "translator-credits");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::StringArrayHandle> AboutDialog::property_artists() 
{
  return Glib::PropertyProxy<Glib::StringArrayHandle>(this, "artists");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle> AboutDialog::property_artists() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::StringArrayHandle>(this, "artists");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> > AboutDialog::property_logo() 
{
  return Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> >(this, "logo");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly< Glib::RefPtr<Gdk::Pixbuf> > AboutDialog::property_logo() const
{
  return Glib::PropertyProxy_ReadOnly< Glib::RefPtr<Gdk::Pixbuf> >(this, "logo");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> AboutDialog::property_logo_icon_name() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "logo-icon-name");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> AboutDialog::property_logo_icon_name() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "logo-icon-name");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/*
Glib::PropertyProxy<bool> AboutDialog::property_wrap_license() 
{
  return Glib::PropertyProxy<bool>(this, "wrap-license");
}
*/
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/*
Glib::PropertyProxy_ReadOnly<bool> AboutDialog::property_wrap_license() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "wrap-license");
}
*/
#endif //GLIBMM_PROPERTIES_ENABLED


#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED


} // namespace Gtk


