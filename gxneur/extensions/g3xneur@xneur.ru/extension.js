/*
 * gnome-shell-extension-g3xneur
 * 
 * Add gxneur icon to Gnome 3 shell area
 * Copyright (C) 2011 Andrew Crew Kuznetsov <andrewcrew@rambler.ru>  
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gnome-shell-ext-g3xneur If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 */


const StatusIconDispatcher = imports.ui.statusIconDispatcher;

function enable() {
    StatusIconDispatcher.STANDARD_TRAY_ICON_IMPLEMENTATIONS['gxneur'] = 'gxneur';
}

function disable() {
    StatusIconDispatcher.STANDARD_TRAY_ICON_IMPLEMENTATIONS['gxneur'] = '';
}

function init() {
   StatusIconDispatcher.STANDARD_TRAY_ICON_IMPLEMENTATIONS['gxneur'] = 'gxneur';
}

function main() {
   StatusIconDispatcher.STANDARD_TRAY_ICON_IMPLEMENTATIONS['gxneur'] = 'gxneur';
}
