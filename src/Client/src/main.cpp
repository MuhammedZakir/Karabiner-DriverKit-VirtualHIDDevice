#include "io_service_client.hpp"
#include "version.hpp"
#include "virtual_hid_device_service_server.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <pqrs/hid.hpp>
#include <pqrs/local_datagram.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/process_info.hpp>
#include <thread>

int main(void) {
  std::signal(SIGINT, SIG_IGN);
  std::signal(SIGTERM, SIG_IGN);
  std::signal(SIGUSR1, SIG_IGN);
  std::signal(SIGUSR2, SIG_IGN);

  pqrs::osx::process_info::enable_sudden_termination();

  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  logger::set_async_rotating_logger("virtual_hid_device_service",
                                    "/var/log/karabiner/virtual_hid_device_service.log",
                                    pqrs::spdlog::filesystem::log_directory_perms_0755);

  logger::get_logger()->info("version {0}", VERSION);

  auto server = std::make_unique<virtual_hid_device_service_server>();

  //
  // Set signal handler
  //

  auto termination_handler = [&server] {
    server = nullptr;
    pqrs::dispatcher::extra::terminate_shared_dispatcher();
    exit(0);
  };

  {
    dispatch_source_t sigint_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGINT, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigint_source, ^{
      logger::get_logger()->info("SIGINT");
      termination_handler();
    });
    dispatch_resume(sigint_source);
  }
  {
    dispatch_source_t sigterm_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigterm_source, ^{
      logger::get_logger()->info("SIGTERM");
      termination_handler();
    });
    dispatch_resume(sigterm_source);
  }

  //
  // Run
  //

  dispatch_main();

  return 0;
}
