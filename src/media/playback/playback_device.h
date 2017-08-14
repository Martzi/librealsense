// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once
#include <atomic>
#include <core/roi.h>
#include <core/extension.h>
#include <core/serialization.h>
#include <media/ros/file_types.h>
#include "core/streaming.h"
#include "archive.h"
#include "concurrency.h"
#include "sensor.h"
#include "playback_sensor.h"

namespace librealsense
{
    /*
     * Internal members meaning:
     *
     *               m_is_started  |     True     |      False
     *  m_is_paused                |              |
     *  ---------------------------|--------------|---------------
     *      True                   |    Paused    |   Stopped
     *      False                  |    Playing   |   Stopped
     *
     */

    class playback_device : public device_interface,
        public extendable_interface,
        public info_container
    {
    public:
        playback_device(std::shared_ptr<context> context, std::shared_ptr<device_serializer::reader> serializer);
        virtual ~playback_device();

        std::shared_ptr<context> get_context() const override;

        sensor_interface& get_sensor(size_t i) override;
        size_t get_sensors_count() const override;
        const sensor_interface& get_sensor(size_t i) const override;
        void hardware_reset() override;
        bool extend_to(rs2_extension extension_type, void** ext) override;
        std::shared_ptr<matcher> create_matcher(const frame_holder& frame) const override;

        void set_frame_rate(double rate);
        void seek_to_time(std::chrono::nanoseconds time);
        rs2_playback_status get_current_status() const;
        uint64_t get_duration() const;
        void pause();
        void resume();
        void set_real_time(bool real_time);
        bool is_real_time() const;
        const std::string& get_file_name() const;
        uint64_t get_position() const;
        signal<playback_device, rs2_playback_status> playback_status_changed;
        platform::backend_device_group get_device_data() const override;
    private:
        void update_time_base(std::chrono::microseconds base_timestamp);
        std::chrono::microseconds calc_sleep_time(std::chrono::microseconds timestamp) const;
        void start();
        void stop();
        void try_looping();
        template <typename T> void do_loop(T op);
        std::map<uint32_t, std::shared_ptr<playback_sensor>> create_playback_sensors(const device_snapshot& device_description);
        void catch_up();
        void register_device_info(const std::shared_ptr<info_interface>& shared);
    private:
        lazy<std::shared_ptr<dispatcher>> m_read_thread;
        std::shared_ptr<device_serializer::reader> m_reader;
        device_snapshot m_device_description;
        std::atomic_bool m_is_started;
        std::atomic_bool m_is_paused;
        std::chrono::high_resolution_clock::time_point m_base_sys_time; // !< System time when reading began (first frame was read)
        std::chrono::microseconds m_base_timestamp; // !< Timestamp of the first frame that has a real timestamp (different than 0)
        std::map<uint32_t, std::shared_ptr<playback_sensor>> m_sensors;
        std::map<uint32_t, std::shared_ptr<playback_sensor>> m_active_sensors;
        std::atomic<double> m_sample_rate;
        std::atomic_bool m_real_time;
        device_serializer::nanoseconds m_prev_timestamp;
        std::shared_ptr<context> m_context;
    };

    MAP_EXTENSION(RS2_EXTENSION_PLAYBACK, playback_device);
}

