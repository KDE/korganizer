/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KORGANIZER_EXPORT_H
#define KORGANIZER_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KORGANIZERPRIVATE_EXPORT
# if defined(MAKE_KORGANIZERPRIVATE_LIB)
   /* We are building this library */
#  define KORGANIZERPRIVATE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORGANIZERPRIVATE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KORG_STDPRINTING_EXPORT
# if defined(MAKE_KORG_STDPRINTING_LIB)
   /* We are building this library */
#  define KORG_STDPRINTING_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORG_STDPRINTING_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KORGANIZER_CORE_EXPORT
# if defined(MAKE_KORGANIZER_CORE_LIB)
   /* We are building this library */
#  define KORGANIZER_CORE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORGANIZER_CORE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KORGANIZER_EVENTVIEWER_EXPORT
# if defined(MAKE_KORGANIZERPRIVATE_LIB)
   /* We are building this library */
#  define KORGANIZER_EVENTVIEWER_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORGANIZER_EVENTVIEWER_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KORGANIZER_INTERFACES_EXPORT
# if defined(MAKE_KORGANIZER_INTERFACES_LIB)
   /* We are building this library */
#  define KORGANIZER_INTERFACES_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORGANIZER_INTERFACES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KORGANIZER_CALENDAR_EXPORT
# if defined(MAKE_KORGANIZER_CALENDAR_LIB)
   /* We are building this library */
#  define KORGANIZER_CALENDAR_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KORGANIZER_CALENDAR_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KCM_KORGANIZER_EXPORT
# if defined(MAKE_KCM_KORGANIZER_LIB)
   /* We are building this library */
#  define KCM_KORGANIZER_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KCM_KORGANIZER_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KORGANIZERPRIVATE_EXPORT_DEPRECATED
#  define KORGANIZERPRIVATE_EXPORT_DEPRECATED KDE_DEPRECATED KORGANIZERPRIVATE_EXPORT
# endif

# ifndef KCM_KORGANIZER_EXPORT_DEPRECATED
#  define KCM_KORGANIZER_EXPORT_DEPRECATED KDE_DEPRECATED KCM_KORGANIZER_EXPORT
# endif

# ifndef KORG_STDPRINTING_EXPORT_DEPRECATED
#  define KORG_STDPRINTING_EXPORT_DEPRECATED KDE_DEPRECATED KORG_STDPRINTING_EXPORT
# endif

# ifndef KORGANIZER_EVENTVIEWER_EXPORT_DEPRECATED
#  define KORGANIZER_EVENTVIEWER_EXPORT_DEPRECATED KDE_DEPRECATED KORGANIZER_EVENTVIEWER_EXPORT
# endif

# ifndef KORGANIZER_CALENDAR_EXPORT_DEPRECATED
#  define KORGANIZER_CALENDAR_EXPORT_DEPRECATED KDE_DEPRECATED KORGANIZER_CALENDAR_EXPORT
# endif

# ifndef KORGANIZER_INTERFACES_EXPORT_DEPRECATED
#  define KORGANIZER_INTERFACES_EXPORT_DEPRECATED KDE_DEPRECATED KORGANIZER_INTERFACES_EXPORT
# endif

# ifndef KORGANIZER_CORE_EXPORT_DEPRECATED
#  define KORGANIZER_CORE_EXPORT_DEPRECATED KDE_DEPRECATED KORGANIZER_CORE_EXPORT
# endif

#endif
