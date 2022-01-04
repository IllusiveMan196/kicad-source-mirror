/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2021 Ola Rinta-Koski
 * Copyright (C) 2021-2022 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <sstream>
#include <font/fontconfig.h>
#include <pgm_base.h>
#include <settings/settings_manager.h>

using namespace fontconfig;

static FONTCONFIG* g_config = nullptr;

inline static FcChar8* wxStringToFcChar8( const wxString& str )
{
    wxScopedCharBuffer const fcBuffer = str.ToUTF8();
    return (FcChar8*) fcBuffer.data();
}


FONTCONFIG::FONTCONFIG()
{
    m_config = FcInitLoadConfigAndFonts();

    wxString configDirPath( Pgm().GetSettingsManager().GetUserSettingsPath() + wxT( "/fonts" ) );
    FcConfigAppFontAddDir( nullptr, wxStringToFcChar8( configDirPath ) );
};


FONTCONFIG& Fontconfig()
{
    if( !g_config )
    {
        FcInit();
        g_config = new FONTCONFIG();
    }

    return *g_config;
}


bool FONTCONFIG::FindFont( const wxString& aFontName, wxString& aFontFile )
{
    FcPattern* pat = FcNameParse( wxStringToFcChar8( aFontName ) );
    FcConfigSubstitute( nullptr, pat, FcMatchPattern );
    FcDefaultSubstitute( pat );

    FcResult   r = FcResultNoMatch;
    FcPattern* font = FcFontMatch( nullptr, pat, &r );

    bool ok = false;

    if( font )
    {
        FcChar8* file = nullptr;

        if( FcPatternGetString( font, FC_FILE, 0, &file ) == FcResultMatch )
        {
            aFontFile = wxString::FromUTF8( (char*) file );
            ok = true;
        }

        FcPatternDestroy( font );
    }

    FcPatternDestroy( pat );
    return ok;
}


void FONTCONFIG::ListFonts( std::vector<std::string>& aFonts )
{
    if( m_fonts.empty() )
    {
        FcPattern*   pat = FcPatternCreate();
        FcObjectSet* os = FcObjectSetBuild( FC_FAMILY, FC_STYLE, FC_LANG, FC_FILE, nullptr );
        FcFontSet*   fs = FcFontList( nullptr, pat, os );

        for( int i = 0; fs && i < fs->nfont; ++i )
        {
            FcPattern* font = fs->fonts[i];
            FcChar8*   file;
            FcChar8*   style;
            FcChar8*   family;

            if( FcPatternGetString( font, FC_FILE, 0, &file ) == FcResultMatch
                && FcPatternGetString( font, FC_FAMILY, 0, &family ) == FcResultMatch
                && FcPatternGetString( font, FC_STYLE, 0, &style ) == FcResultMatch )
            {
                std::ostringstream s;
                s << family;

                std::string theFile( reinterpret_cast<char *>( file ) );
                std::string theFamily( reinterpret_cast<char *>( family ) );
                std::string theStyle( reinterpret_cast<char *>( style ) );
                FONTINFO    fontInfo( theFile, theStyle, theFamily );

                if( theFamily.length() > 0 && theFamily.front() == '.' )
                    continue;

                auto it = m_fonts.find( theFamily );

                if( it == m_fonts.end() )
                    m_fonts.insert( std::pair<std::string, FONTINFO>( theFamily, fontInfo ) );
                else
                    it->second.Children().push_back( fontInfo );
            }
        }

        if( fs )
            FcFontSetDestroy( fs );
    }

    for( const std::pair<const std::string, FONTINFO>& entry : m_fonts )
        aFonts.push_back( entry.second.Family() );
}
