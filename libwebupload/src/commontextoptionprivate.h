 
/*
 * Web Upload Engine -- MeeGo social networking uploads
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Contact: Jukka Tiihonen <jukka.tiihonen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#ifndef _WEBUPLOAD_COMMON_TEXT_OPTION_PRIVATE_H_
#define _WEBUPLOAD_COMMON_TEXT_OPTION_PRIVATE_H_


namespace WebUpload {

    /*!
        \class CommonTextOptionPrivate
        \brief Private class for CommonTextOption
        \author Jukka Tiihonen <jukka.t.tiihonen@nokia.com>
     */
    class WEBUPLOAD_EXPORT CommonTextOptionPrivate {
    public:
        CommonTextOptionPrivate (CommonTextOption * parent);
        virtual ~CommonTextOptionPrivate();
        
        /*!
          \brief Get default localized tooltip for the text widget
          \return Default localized tooltip string
         */
        QString defaultTooltip () const;
        
        //! Instance of public class
        CommonTextOption * m_parent;
        
        //! Is the text widget multiline?
        bool m_isMultiline;
        
        //! If this option should be prefilled for user
        bool m_prefill;
        
        //! Tooltip string to show in the text entry widget if there is no
        //  value there.
        QString m_tooltip;
    
    };
}

#endif
