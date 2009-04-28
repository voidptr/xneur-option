/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Copyright (C) 2006-2009 XNeur Team
 *
 */

#ifndef _XSWITCHLANG_H_
#define _XSWITCHLANG_H_

int  get_active_keyboard_group(void);
int  get_cur_lang(void);
void switch_lang(int new_lang);
void switch_group(int new_group);
int  get_keyboard_groups_count(void);
int  print_keyboard_groups(void);
void set_next_keyboard_group(void);

#endif /* _XSWITCHLANG_H_ */
