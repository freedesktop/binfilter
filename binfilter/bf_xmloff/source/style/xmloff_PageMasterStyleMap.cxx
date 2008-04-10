/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmloff_PageMasterStyleMap.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
namespace binfilter {

using namespace ::binfilter::xmloff::token;

#define MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context }
//______________________________________________________________________________

const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
// page master
    MAP( "PageStyleLayout",         XML_NAMESPACE_STYLE,    XML_PAGE_USAGE,                 XML_PM_TYPE_PAGESTYLELAYOUT | MID_FLAG_SPECIAL_ITEM,    CTF_PM_PAGEUSAGE ),
    MAP( "Width",                   XML_NAMESPACE_FO,       XML_PAGE_WIDTH,                 XML_TYPE_MEASURE,                                       0 ),
    MAP( "Height",                  XML_NAMESPACE_FO,       XML_PAGE_HEIGHT,                XML_TYPE_MEASURE,                                       0 ),
    MAP( "NumberingType",           XML_NAMESPACE_STYLE,    XML_NUM_FORMAT,                 XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY,        0 ),
    MAP( "NumberingType",           XML_NAMESPACE_STYLE,    XML_NUM_LETTER_SYNC,            XML_PM_TYPE_NUMLETTERSYNC,                              0 ),
    MAP( "PrinterPaperTray",        XML_NAMESPACE_STYLE,    XML_PAPER_TRAY_NAME,            XML_TYPE_STRING | MID_FLAG_PROPERTY_MAY_EXCEPT,         0 ),
    MAP( "IsLandscape",             XML_NAMESPACE_STYLE,    XML_PRINT_ORIENTATION,          XML_PM_TYPE_PRINTORIENTATION,                           0 ),
    MAP( "TopMargin",               XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,                                       0 ),
    MAP( "BottomMargin",            XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,                                       0 ),
    MAP( "LeftMargin",              XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,                                       0 ),
    MAP( "RightMargin",             XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,                                       0 ),
    MAP( "TopBorder",               XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,                                        CTF_PM_BORDERALL ),
    MAP( "TopBorder",               XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,                                        CTF_PM_BORDERTOP ),
    MAP( "BottomBorder",            XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,                                        CTF_PM_BORDERBOTTOM ),
    MAP( "LeftBorder",              XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERLEFT ),
    MAP( "RightBorder",             XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,                                        CTF_PM_BORDERRIGHT ),
    MAP( "TopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHALL ),
    MAP( "TopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHTOP ),
    MAP( "BottomBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHBOTTOM ),
    MAP( "LeftBorder",              XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHLEFT ),
    MAP( "RightBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHRIGHT ),
    MAP( "TopBorderDistance",       XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,                                       CTF_PM_PADDINGALL ),
    MAP( "TopBorderDistance",       XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,                                       CTF_PM_PADDINGTOP ),
    MAP( "BottomBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,                                       CTF_PM_PADDINGBOTTOM ),
    MAP( "LeftBorderDistance",      XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,                                       CTF_PM_PADDINGLEFT ),
    MAP( "RightBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,                                       CTF_PM_PADDINGRIGHT ),
    MAP( "ShadowFormat",            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,                                   0 ),
    MAP( "BackColor",               XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    0 ),
    MAP( "BackTransparent",         XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      0 ),
    MAP( "BackGraphicLocation",     XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_GRAPHICPOSITION ),
    MAP( "BackGraphicFilter",       XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_GRAPHICFILTER ),
    MAP( "BackGraphicURL",          XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_GRAPHICURL ),
    MAP( "PrintAnnotations",        XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTANNOTATIONS | MID_FLAG_MULTI_PROPERTY,                         CTF_PM_PRINT_ANNOTATIONS ),
    MAP( "PrintCharts",             XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTCHARTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,   CTF_PM_PRINT_CHARTS ),
    MAP( "PrintDrawing",            XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTDRAWING | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_DRAWING ),
    MAP( "PrintFormulas",           XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTFORMULAS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, CTF_PM_PRINT_FORMULAS ),
    MAP( "PrintGrid",               XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTGRID | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,     CTF_PM_PRINT_GRID ),
    MAP( "PrintHeaders",            XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTHEADERS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_HEADERS ),
    MAP( "PrintObjects",            XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTOBJECTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_OBJECTS ),
    MAP( "PrintZeroValues",         XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTZEROVALUES | MID_FLAG_MERGE_ATTRIBUTE,                         CTF_PM_PRINT_ZEROVALUES ),
    MAP( "PrintDownFirst",          XML_NAMESPACE_STYLE,    XML_PRINT_PAGE_ORDER,           XML_PM_TYPE_PRINTPAGEORDER,                             0 ),
    MAP( "FirstPageNumber",         XML_NAMESPACE_STYLE,    XML_FIRST_PAGE_NUMBER,          XML_PM_TYPE_FIRSTPAGENUMBER,                            0 ),
    MAP( "PageScale",               XML_NAMESPACE_STYLE,    XML_SCALE_TO,                   XML_TYPE_NUMBER16,                                      CTF_PM_SCALETO ),
    MAP( "ScaleToPages",            XML_NAMESPACE_STYLE,    XML_SCALE_TO_PAGES,             XML_TYPE_NUMBER16,                                      CTF_PM_SCALETOPAGES ),
    MAP( "CenterHorizontally",      XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_HORIZONTAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "CenterVertically",        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_VERTICAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "TextColumns",             XML_NAMESPACE_STYLE,    XML_COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_PM_TEXTCOLUMNS ),
    MAP( "RegisterModeActive",      XML_NAMESPACE_STYLE,    XML_REGISTER_TRUE,  XML_TYPE_BOOL, 0 ),
    MAP( "RegisterParagraphStyle",  XML_NAMESPACE_STYLE,    XML_REGISTER_TRUTH_REF_STYLE_NAME,  XML_TYPE_STRING| MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_PM_REGISTER_STYLE ),
    MAP( "WritingMode",                 XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_TYPE_TEXT_WRITING_MODE, 0 ),

    MAP( "GridColor", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_COLOR, XML_TYPE_COLOR, 0 ),
    MAP( "GridLines", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_LINES, XML_TYPE_NUMBER16, 0 ),
    MAP( "GridBaseHeight", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_TYPE_MEASURE, 0 ),
    MAP( "GridRubyHeight", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_TYPE_MEASURE, 0 ),
    MAP( "GridMode", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_MODE, XML_TYPE_LAYOUT_GRID_MODE, 0 ),
    MAP( "RubyBelow", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_BELOW, XML_TYPE_BOOL, 0 ),
    MAP( "GridPrint", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_PRINT, XML_TYPE_BOOL, 0 ),
    MAP( "GridDisplay", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_DISPLAY, XML_TYPE_BOOL, 0 ),

    MAP( "UserDefinedAttributes",   XML_NAMESPACE_TEXT,     XML_XMLNS,                      XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

// footnote
    MAP( "FootnoteHeight",          XML_NAMESPACE_STYLE,    XML_FOOTNOTE_MAX_HEIGHT, XML_TYPE_MEASURE, CTF_PM_FTN_HEIGTH ),
    MAP( "FootnoteLineAdjust",      XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_TEXT_HORIZONTAL_ADJUST|MID_FLAG_SPECIAL_ITEM,  CTF_PM_FTN_LINE_ADJUST ),
    MAP( "FootnoteLineColor",       XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM,       CTF_PM_FTN_LINE_COLOR ),
    MAP( "FootnoteLineDistance",    XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,CTF_PM_FTN_DISTANCE ),
    MAP( "FootnoteLineRelativeWidth", XML_NAMESPACE_STYLE,  XML__EMPTY,     XML_TYPE_PERCENT8|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_WIDTH ),
    MAP( "FootnoteLineTextDistance", XML_NAMESPACE_STYLE,   XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM, CTF_PM_FTN_LINE_DISTANCE ),
    MAP( "FootnoteLineWeight",      XML_NAMESPACE_STYLE,    XML_FOOTNOTE_SEP,   XML_TYPE_MEASURE16|MID_FLAG_ELEMENT_ITEM,   CTF_PM_FTN_LINE_WEIGTH ),

// header
    MAP( "HeaderHeight",                XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_HEADERHEIGHT ),
    MAP( "HeaderHeight",                XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_HEADERMINHEIGHT ),
    MAP( "HeaderIsDynamicHeight",       XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_HEADERDYNAMIC ),
    MAP( "HeaderLeftMargin",            XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    MAP( "HeaderRightMargin",           XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    MAP( "HeaderBodyDistance",          XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    MAP( "HeaderTopBorder",             XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_HEADERBORDERALL ),
    MAP( "HeaderTopBorder",             XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_HEADERBORDERTOP ),
    MAP( "HeaderBottomBorder",          XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_HEADERBORDERBOTTOM ),
    MAP( "HeaderLeftBorder",            XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERLEFT ),
    MAP( "HeaderRightBorder",           XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_HEADERBORDERRIGHT ),
    MAP( "HeaderTopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHALL ),
    MAP( "HeaderTopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHTOP ),
    MAP( "HeaderBottomBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHBOTTOM ),
    MAP( "HeaderLeftBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHLEFT ),
    MAP( "HeaderRightBorder",           XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHRIGHT ),
    MAP( "HeaderTopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGALL ),
    MAP( "HeaderTopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGTOP ),
    MAP( "HeaderBottomBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGBOTTOM ),
    MAP( "HeaderLeftBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGLEFT ),
    MAP( "HeaderRightBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGRIGHT ),
    MAP( "HeaderShadowFormat",          XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_HEADERFLAG ),
    MAP( "HeaderBackColor",             XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_HEADERFLAG ),
    MAP( "HeaderBackTransparent",       XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_HEADERFLAG ),
    MAP( "HeaderBackGraphicLocation",   XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_HEADERGRAPHICPOSITION ),
    MAP( "HeaderBackGraphicFilter",     XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_HEADERGRAPHICFILTER ),
    MAP( "HeaderBackGraphicURL",        XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_HEADERGRAPHICURL ),
    MAP( "HeaderDynamicSpacing",        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,            XML_TYPE_BOOL,          CTF_PM_HEADERFLAG ),

// footer
    MAP( "FooterHeight",                XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_FOOTERHEIGHT ),
    MAP( "FooterHeight",                XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERMINHEIGHT ),
    MAP( "FooterIsDynamicHeight",       XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_FOOTERDYNAMIC ),
    MAP( "FooterLeftMargin",            XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    MAP( "FooterRightMargin",           XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    MAP( "FooterBodyDistance",          XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    MAP( "FooterTopBorder",             XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERALL ),
    MAP( "FooterTopBorder",             XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERTOP ),
    MAP( "FooterBottomBorder",          XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERBOTTOM ),
    MAP( "FooterLeftBorder",            XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERLEFT ),
    MAP( "FooterRightBorder",           XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERRIGHT ),
    MAP( "FooterTopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHALL ),
    MAP( "FooterTopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHTOP ),
    MAP( "FooterBottomBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHBOTTOM ),
    MAP( "FooterLeftBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHLEFT ),
    MAP( "FooterRightBorder",           XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHRIGHT ),
    MAP( "FooterTopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGALL ),
    MAP( "FooterTopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGTOP ),
    MAP( "FooterBottomBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGBOTTOM ),
    MAP( "FooterLeftBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGLEFT ),
    MAP( "FooterRightBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGRIGHT ),
    MAP( "FooterShadowFormat",          XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_FOOTERFLAG ),
    MAP( "FooterBackColor",             XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_FOOTERFLAG ),
    MAP( "FooterBackTransparent",       XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_FOOTERFLAG ),
    MAP( "FooterBackGraphicLocation",   XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_FOOTERGRAPHICPOSITION ),
    MAP( "FooterBackGraphicFilter",     XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_FOOTERGRAPHICFILTER ),
    MAP( "FooterBackGraphicURL",        XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_FOOTERGRAPHICURL ),
    MAP( "FooterDynamicSpacing",        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,           XML_TYPE_BOOL,          CTF_PM_FOOTERFLAG ),

    { 0L }
};

}//end of namespace binfilter
