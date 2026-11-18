/**
 * @file    comlink_ids.h
 * @brief   Service instance and message identifiers of the application.
 * @author  CYK-Dot <chenyukai@xmurcs.cn>
 * @date    2026-11-18
 *
 * SPDX-FileCopyrightText: 2026 CYK-Dot <chenyukai@xmurcs.cn>
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef COMLINK_IDS_H
#define COMLINK_IDS_H

/** @brief Service instance identifiers deployed by the application. */
enum {
    COMLINK_ASVCIID_USBTEST = 0, /**< USB test service. */
};

/** @brief Message identifiers consumed by the USB test service. */
enum {
    COMLINK_MSG_USB_TASK = 1, /**< Run one USB device task pass. */
    COMLINK_MSG_USB_HELLO,    /**< Send the periodic hello string. */
    COMLINK_MSG_LED,          /**< Toggle the status LED. */
};

#endif /* COMLINK_IDS_H */
