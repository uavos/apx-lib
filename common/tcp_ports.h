/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// TCP ports to listen on for HTTP servers

#define TCP_PORT_SERVER 9280 // GCS http server
#define TCP_PORT_SIM 9288    // simulator http server

// UDP ports to listen on

// GCS discover service
#define UDP_PORT_DISCOVER 9200

// GCS downlink/telemetry service
#define UDP_PORT_GCS_TLM 9210

// AP uplink service
#define UDP_PORT_AP_UPLINK 9211

// SIM controls input service
#define UDP_PORT_SIM_CTR 9220

// AP sim sensors input service
#define UDP_PORT_AP_SNS 9221

// GCS sns/ctr forward gate service (gcs sim plugin)
#define UDP_PORT_GCS_SIM 9222
