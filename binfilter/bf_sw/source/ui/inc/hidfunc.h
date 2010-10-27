/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#define HID_EXIT    1017
#define HID_NEW_WINDOW  1701
#define HID_ARRANGE_CASCADE_WIN 1703
#define HID_ARRANGE_HORZ_WIN    1704
#define HID_ARRANGE_VERT_WIN    1705
#define HID_ARRANGE_ALL_WIN 1702
#define HID_SHOW_HELP_INDEX 1802
#define HID_NEW_FILE    1005
#define HID_NEW_FILE_DEFAULT    1024
#define HID_CLOSE_FILE  1001
#define HID_DOC_MGR_DLG 1032
#define HID_CONFIG_TOOLBOX  1629
#define HID_CONFIG_MENU 1630
#define HID_CONFIG_KEY  1631
#define HID_TEMPLATE_PI 1657
#define HID_ADDR_PI 1658
#define HID_AUTO_FORMAT 1651
#define HID_NAVIGATION_PI   1656
#define HID_NAVIGATION_PI_GOTO_PAGE 1659
#define HID_HELP_PI 1655
#define HID_FORMAT_DROPCAPS 1454
#define HID_CONFIG_VIEWOPTIONS  1635
#define HID_CONFIG_COLORS   1636
#define HID_TOOLBOX 1213
#define HID_RIBBON  1210
#define HID_RULER   1211
#define HID_VLINEAL 1216
#define HID_VSCROLLBAR  1217
#define HID_HSCROLLBAR  1218
#define HID_SAVE_FILE   1013
#define HID_SAVE_ALL    1023
#define HID_SAVE_FILE_AS    1012
#define HID_OPEN_LASTVERSION    1009
#define HID_STATUSBAR   1212
#define HID_ABOUT_DLG   1806
#define HID_SEND_MAIL   1022
#define HID_OPEN_FILE   1007
#define HID_INSERT_FILE 1309
#define HID_DOCSTAT_DLG 1020
#define HID_LOAD_TEMPLATE_DLG   1021
#define HID_ESCAPE  1941
#define HID_PLAY_MACRO_DLG  1624
#define HID_RECORD_MACRO_DLG    1622
#define HID_ASSIGN_LOCAL_MACRO_DLG  1632
#define HID_COMPILE_MACRO   1633
#define HID_CREATE_DOCUMENT 1019
#define HID_PAGEUP  1937
#define HID_PAGEDOWN    1938
#define HID_SET_LEFT_PARA   1418
#define HID_SET_RIGHT_PARA  1419
#define HID_SET_CENTER_PARA 1420
#define HID_SET_JUSTIFY_PARA    1421
#define HID_SET_ADJUST  1428
#define HID_FORMAT_RESET    1469
#define HID_INDENT_TO_TABSTOP   1417
#define HID_UNINDENT_TO_TABSTOP 1431
#define HID_SET_BOLD    1405
#define HID_SET_ITALIC  1406
#define HID_SET_UNDERLINE   1409
#define HID_SET_STRIKEOUT   1410
#define HID_SET_OUTLINE 1408
#define HID_SET_SHADOW  1407
#define HID_SET_SUPER_SCRIPT    1411
#define HID_SET_SUB_SCRIPT  1412
#define HID_SET_COLOR   1413
#define HID_SET_CASEMAP 1414
#define HID_SET_LANGUAGE    1415
#define HID_SET_KERNING 1416
#define HID_SET_FONT    1401
#define HID_SET_FONT_SIZE   1402
#define HID_GROW_FONT_SIZE  1403
#define HID_SHRINK_FONT_SIZE    1404
#define HID_SET_LINE_SPACE_1    1422
#define HID_SET_LINE_SPACE_15   1423
#define HID_SET_LINE_SPACE_2    1424
#define HID_SET_PROP_LINE_SPACE 1540
#define HID_SET_ABS_LINE_SPACE  1541
#define HID_INSERT_SYMBOL   1328
#define HID_INSERT_OBJECT_DLG   1322
#define HID_EDIT_OBJECT 1110
#define HID_EDIT_LINK_DLG   1109
#define HID_EDIT_FILE_INFO_DLG  1107
#define HID_EDIT_IDX_ENTRY_DLG  1123
#define HID_NUM_BULLET_DOWN 1130
#define HID_NUM_BULLET_MOVEUP   1134
#define HID_NUM_BULLET_MOVEDOWN 1135
#define HID_NUM_BULLET_UP   1131
#define HID_NUM_BULLET_PREV 1132
#define HID_NUM_BULLET_NEXT 1133
#define HID_NUM_BULLET_OUTLINE_MOVEDOWN 1142
#define HID_NUM_BULLET_OUTLINE_MOVEUP   1141
#define HID_NUM_BULLET_OUTLINE_UP   1140
#define HID_NUM_BULLET_OUTLINE_DOWN 1139
#define HID_NUM_BULLET_NONUM    1136
#define HID_NUM_NUMBERING_ON    1144
#define HID_NUM_BULLET_ON   1138
#define HID_NUM_BULLET_OFF  1137
#define HID_INSERT_IDX_ENTRY_DLG    1335
#define HID_POSTIT  1329
#define HID_INSERT_CAPTION  1310
#define HID_INSERT_IDX  1336
#define HID_INSERT_USERIDX  1337
#define HID_INSERT_CNTNTIDX 1338
#define HID_INSERT_IDX_ENTRY    1340
#define HID_INSERT_USERIDX_ENTRY    1341
#define HID_INSERT_CNTNTIDX_ENTRY   1342
#define HID_INSERT_FRAME    1334
#define HID_INSERT_FRAME_INTERACT   1333
#define HID_INSERT_SOFT_HYPHEN  1350
#define HID_INSERT_HARD_SPACE   1351
#define HID_FORMAT_FRAME_DLG    1456
#define HID_FRAME_TO_TOP    1470
#define HID_FRAME_TO_BOTTOM 1471
#define HID_FRAME_NOWRAP    1472
#define HID_FRAME_WRAP  1473
#define HID_FRAME_WRAPTHRU  1474
#define HID_FRAME_ALIGN_HORZ_LEFT   1475
#define HID_FRAME_ALIGN_HORZ_RIGHT  1476
#define HID_FRAME_ALIGN_HORZ_CENTER 1477
#define HID_FRAME_ALIGN_VERT_TOP    1478
#define HID_FRAME_ALIGN_VERT_BOTTOM 1479
#define HID_FRAME_ALIGN_VERT_CENTER 1480
#define HID_EDIT_FIELD  1104
#define HID_SET_FIELD_VALUE 1112
#define HID_INSERT_FIELD    1308
#define HID_INSERT_MERGEFIELD_DLG   1320
#define HID_EXECUTE_MACROFIELD  1127
#define HID_UPDATE_FIELDS   1126
#define HID_UPDATE_INPUTFIELDS  1143
#define HID_CALC_TABLE  1129
#define HID_INSERT_BREAK_DLG    1304
#define HID_INSERT_BOOKMARK 1302
#define HID_DELETE_BOOKMARK 1301
#define HID_INSERT_GRAFIC   1315
#define HID_FORMAT_TABLE_DLG    1460
#define HID_INSERT_TABLE    1330
#define HID_INSERT_COLS 1332
#define HID_PAGE_STYLE_SET_COLS 1538
#define HID_CONVERT_TEXT_TABLE  1500
#define HID_TABLE_INSERT_ROW    1501
#define HID_TABLE_INSERT_COL    1502
#define HID_TABLE_DELETE_ROW    1503
#define HID_TABLE_DELETE_COL    1504
#define HID_TABLE_SPLIT_CELLS   1505
#define HID_TABLE_MERGE_CELLS   1506
#define HID_TABLE_SET_ROW_HEIGHT    1507
#define HID_TABLE_SET_COL_WIDTH 1508
#define HID_TABLE_SET_ULSPACE   1509
#define HID_TABLE_SET_GRID  1510
#define HID_TABLE_SET_SHADOW    1511
#define HID_FORMAT_CHAR_DLG 1441
#define HID_FORMAT_PAGE_DLG 1452
#define HID_CHANGE_PAGENUM  1634
#define HID_FORMAT_PARA_DLG 1443
#define HID_FORMAT_TAB_DLG  1445
#define HID_FORMAT_BORDER_DLG   1447
#define HID_FORMAT_BORDER   1439
#define HID_FORMAT_SHADOW   1438
#define HID_FORMAT_BACKGROUND   1448
#define HID_FORMAT_BACKGROUND_DLG   1450
#define HID_SET_FRM_SIZE    1481
#define HID_SET_FRM_POSITION    1482
#define HID_SET_FRM_ANCHOR  1483
#define HID_SET_FRM_WRAP    1484
#define HID_SET_FRM_DIST    1485
#define HID_SET_FRM_OPTIONS 1486
#define HID_SET_FRM_MACRO   1494
#define HID_SET_FRM_COLUMNS 1495
#define HID_SET_FRM_COLUMN_WIDTH    1496
#define HID_SET_GRF_MIRROR  1488
#define HID_SET_TABLE_WIDTH 1489
#define HID_SET_TABLE_ALIGN 1490
#define HID_TABLE_SET_READ_ONLY_CELLS   1517
#define HID_TABLE_UNSET_READ_ONLY_CELLS 1519
#define HID_NEW_STYLE_BY_EXAMPLE    1462
#define HID_UPDATE_STYLES_DIRECT    1497
#define HID_UPDATE_STYLE_BY_EXAMPLE 1463
#define HID_STYLE_SHEET_FRAME_DLG   1464
#define HID_STYLE_SHEET_DOC_DLG 1465
#define HID_SET_STANDARD_FONTS  1442
#define HID_COPY    1102
#define HID_PASTE   1113
#define HID_PASTESPECIAL    1114
#define HID_CUT 1103
#define HID_FLIP_HORZ_GRAFIC    1425
#define HID_FLIP_VERT_GRAFIC    1426
#define HID_FORMAT_GRAFIC_DLG   1458
#define HID_VIEW_BOUNDS 1214
#define HID_VIEW_FIELDS 1215
#define HID_VIEW_META_CHARS 1224
#define HID_VIEW_MARKS  1225
#define HID_VIEW_FIELDNAME  1226
#define HID_VIEW_TABLEGRID  1227
#define HID_SEARCH  1117
#define HID_REPEAT_SEARCH   1150
#define HID_REPLACE 1118
#define HID_GOTO    1111
#define HID_SETUP_PRINTER_DLG   1015
#define HID_PRINT_FILE  1010
#define HID_PRINT_FILE_DEFAULT  1025
#define HID_FAX 1028
#define HID_PRINT_FILE_OPTIONS  1011
#define HID_THESAURUS_DLG   1604
#define HID_SPELLING_DLG    1601
#define HID_HYPHENATE_OPT_DLG   1607
#define HID_MERGE_FILE_DLG  1004
#define HID_SELECT_DATABASE 1030
#define HID_DOC_INFO_DLG    1031
#define HID_MERGE_FILE  1003
#define HID_MERGE_RECORD    1327
#define HID_INSERT_RECORD   1326
#define HID_NUMBERING_OUTLINE_DLG   1612
#define HID_NUMBER_BULLETS  1121
#define HID_ZOOM    1231
#define HID_SORTING 1613
#define HID_SORTING_DLG 1614
#define HID_CALCULATE   1615
#define HID_REFRESH_VIEW    1201
#define HID_CHAR_LEFT   1901
#define HID_CHAR_RIGHT  1902
#define HID_LINE_UP 1903
#define HID_LINE_DOWN   1904
#define HID_START_OF_LINE   1905
#define HID_END_OF_LINE 1906
#define HID_START_OF_DOCUMENT   1907
#define HID_END_OF_DOCUMENT 1908
#define HID_SELECT_WORD 1943
#define HID_SELECT_ALL  1119
#define HID_START_OF_NEXT_PAGE  1909
#define HID_END_OF_NEXT_PAGE    1910
#define HID_START_OF_PREV_PAGE  1911
#define HID_END_OF_PREV_PAGE    1912
#define HID_START_OF_PAGE   1913
#define HID_END_OF_PAGE 1914
#define HID_NEXT_WORD   1921
#define HID_START_OF_PARA   1919
#define HID_END_OF_PARA 1920
#define HID_PREV_WORD   1922
#define HID_NEXT_SENT   1923
#define HID_PREV_SENT   1924
#define HID_SET_INS_MODE    1936
#define HID_SET_ADD_MODE    1939
#define HID_SET_EXT_MODE    1940
#define HID_START_TABLE 1947
#define HID_END_TABLE   1948
#define HID_NEXT_TABLE  1949
#define HID_PREV_TABLE  1950
#define HID_TABLE_SELECT_ALL    1515
#define HID_TABLE_SELECT_COL    1514
#define HID_TABLE_SELECT_ROW    1513
#define HID_START_OF_COLUMN 1917
#define HID_END_OF_COLUMN   1918
#define HID_START_OF_NEXT_COLUMN    1951
#define HID_END_OF_NEXT_COLUMN  1952
#define HID_START_OF_PREV_COLUMN    1953
#define HID_END_OF_PREV_COLUMN  1954
#define HID_FOOTNOTE_TO_ANCHOR  1955
#define HID_NEXT_FOOTNOTE   1956
#define HID_PREV_FOOTNOTE   1957
#define HID_CNTNT_TO_NEXT_FRAME 1958
#define HID_FRAME_TO_ANCHOR 1959
#define HID_TO_HEADER   1960
#define HID_TO_FOOTER   1961
#define HID_IDX_MARK_TO_IDX 1962
#define HID_DELETE  1925
#define HID_BACKSPACE   1926
#define HID_DELETE_SENT 1927
#define HID_DELETE_BACK_SENT    1928
#define HID_DELETE_WORD 1929
#define HID_DELETE_BACK_WORD    1930
#define HID_DELETE_LINE 1931
#define HID_DELETE_BACK_LINE    1932
#define HID_DELETE_PARA 1933
#define HID_DELETE_BACK_PARA    1934
#define HID_DELETE_WHOLE_LINE   1935
#define HID_INSERT_BREAK    1303
#define HID_INSERT_PAGEBREAK    1323
#define HID_INSERT_LINEBREAK    1318
#define HID_INSERT_COLUMN_BREAK 1305
#define HID_INSERT_FOOTNOTE 1311
#define HID_INSERT_FOOTNOTE_DLG 1312
#define HID_FORMAT_FOOTNOTE_DLG 1468
#define HID_UNDO    1120
#define HID_REDO    1116
#define HID_REPEAT  1122
#define HID_REPAGINATE  1161
#define HID_SHOW_UPDATE_HELP    1805
#define HID_SHOW_KEYBOARD_HELP  1803
#define HID_SHOW_HELP_MANUAL    1804
#define HID_GLOSSARY_DLG    1620
#define HID_EXPAND_GLOSSARY 1628
#define HID_INSERT_GLOSSARY 1640
#define HID_AUTO_CORRECT_DLG    1650
#define HID_AUTO_CORRECT    1649
#define HID_CONFIG_DLG  1627
#define HID_INSERT_STRING   1331
#define HID_SET_CHAR_STYLE  1491
#define HID_SET_PARA_STYLE  1492
#define HID_SET_PAGE_STYLE  1493
#define HID_SET_LRMARGIN    1429
#define HID_SET_ULMARGIN    1430
#define HID_SET_HYPHEN_ZONE 1432
#define HID_SET_PAGE_BREAK  1433
#define HID_FORMAT_PAGE 1451
#define HID_SET_KEEP_TOGETHER   1434
#define HID_SET_WIDOW   1436
#define HID_SET_ORPHAN  1437
#define HID_BUFFER_UPDATE   1124
#define HID_FORMAT_TAB  1444
#define HID_IS_END_OF_PARA  2000
#define HID_IS_END_OF_WORD  2000
#define HID_IS_END_OF_DOC   2000
#define HID_IS_START_OF_PARA    2000
#define HID_IS_START_OF_WORD    2000
#define HID_IS_START_OF_DOC 2000
#define HID_EDIT_FORMULA    1128
#define HID_ENVELOP 2050
#define HID_LABEL   2051
#define HID_AGENDA_WIZZARD  1663
#define HID_MEMO_WIZZARD    1662
#define HID_FAX_WIZZARD 1661
#define HID_LETTER_WIZZARD  1660
#define HID_LAUNCH_REGISTRY 1002

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
