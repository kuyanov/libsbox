/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_CONTAINER_H
#define LIBSBOX_CONTAINER_H

#include "context_manager.h"
#include "shared_barrier.h"
#include "task_data.h"
#include "cgroup_controller.h"

#include <filesystem>
#include <json.hpp>
#include <sys/resource.h>

namespace fs = std::filesystem;

class Container : public ContextManager {
public:
    Container(int id, bool persistent);
    ~Container() = default;

    pid_t start();
    void get_task_from_json(const nlohmann::json &json_task);
    nlohmann::json results_to_json();

    int get_id();
    pid_t get_pid();
    SharedBarrier *get_barrier();

    [[noreturn]]
    void die(const std::string &error) override;
    void terminate() override;
private:
    int id_;
    pid_t pid_{};
    bool persistent_;
    SharedMemoryObject<TaskData> task_data_{};
    SharedBarrier barrier_{2};
    fs::path root_;
    fs::path work_dir_;

    CgroupController *cpuacct_controller_ = nullptr;
    CgroupController *memory_controller_ = nullptr;
    pid_t slave_pid_ = -1;
    struct timeval run_start_ = {};

    static int clone_callback(void *ptr);
    void serve();
    void prepare();
    void prepare_root();
    void disable_ipcs();
    void wait_for_slave();
    void kill_all();
    void reset_wall_clock();
    long get_wall_clock();
    long get_time_usage_ms();
    long get_time_usage_sys_ms();
    long get_time_usage_user_ms();
    long get_memory_usage_kb();
    bool is_oom_killed();

    int exec_fd_ = -1;

    [[noreturn]]
    void slave();
    void freopen_fds();
    void dup2_fds();
    void close_all_fds();
    void setup_rlimits();
    void set_rlimit_ext(const char *res_name, int res, rlim_t limit);
    void setup_credentials();
};

#endif //LIBSBOX_CONTAINER_H