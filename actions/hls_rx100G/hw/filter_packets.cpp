/*
 * Copyright 2019 Paul Scherrer Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hw_action_rx100G.h"

void check_for_trigger(DATA_STREAM &in, DATA_STREAM &out, uint8_t expected_triggers, ap_uint<24> frames_per_trigger) {
	uint8_t encountered_triggers = 0;
	ap_uint<24> frame_number_last_trigger = 0;
	ap_uint<1> trigger_set = 0; // This is only for beginning, to filter situation, where no filter was used

	data_packet_t packet_in;
	in.read(packet_in);

	while (packet_in.exit == 0) {
#pragma HLS PIPELINE
		if (expected_triggers == 0) out.write(packet_in);
		else {
			ap_int<25> delta = packet_in.frame_number - frame_number_last_trigger;

			if ((trigger_set == 1) && (delta >= 0) && (delta < frames_per_trigger)) {
				packet_in.frame_number = (encountered_triggers - 1 ) * frames_per_trigger + delta;
				out.write(packet_in);
			} else if ((packet_in.trigger == 1) && (encountered_triggers < expected_triggers)) {
				trigger_set = 1;
				frame_number_last_trigger = packet_in.frame_number;
				encountered_triggers ++;
				packet_in.frame_number = (encountered_triggers - 1 ) * frames_per_trigger;
				out.write(packet_in);
			}
		}
		in.read(packet_in);
	}
	out.write(packet_in);
}