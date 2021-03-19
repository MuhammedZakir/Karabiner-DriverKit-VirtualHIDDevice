#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp"
#include "version.hpp"
#include <os/log.h>

#define LOG_PREFIX "Karabiner-DriverKit-VirtualHIDDeviceUserClient " KARABINER_DRIVERKIT_VERSION

namespace {
kern_return_t createIOMemoryDescriptor(IOUserClientMethodArguments* arguments, IOMemoryDescriptor** memory) {
  if (!memory) {
    return kIOReturnBadArgument;
  }

  *memory = nullptr;

  if (arguments->structureInput) {
    auto kr = IOBufferMemoryDescriptorUtility::createWithBytes(arguments->structureInput->getBytesNoCopy(),
                                                               arguments->structureInput->getLength(),
                                                               memory);
    if (kr != kIOReturnSuccess) {
      return kr;
    }
  } else if (arguments->structureInputDescriptor) {
    *memory = arguments->structureInputDescriptor;
    (*memory)->retain();
  }

  return kIOReturnSuccess;
}
} // namespace

struct org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars {
  org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot* provider;
};

bool org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  OSSafeReleaseNULL(ivars->provider);

  IOSafeDeleteNULL(ivars, org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars, 1);

  super::free();
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, Start) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start");

  ivars->provider = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, provider);
  if (!ivars->provider) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " provider is not org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot");
    return kIOReturnError;
  }

  ivars->provider->retain();

  {
    auto kr = Start(provider, SUPERDISPATCH);
    if (kr != kIOReturnSuccess) {
      Stop(provider, SUPERDISPATCH);
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start failed");
      return kr;
    }
  }

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::ExternalMethod(uint64_t selector,
                                                                                      IOUserClientMethodArguments* arguments,
                                                                                      const IOUserClientMethodDispatch* dispatch,
                                                                                      OSObject* target,
                                                                                      void* reference) {
  switch (pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method(selector)) {
    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::driver_version:
      if (arguments->scalarOutput && arguments->scalarOutputCount > 0) {
        arguments->scalarOutput[0] = KARABINER_DRIVERKIT_VERSION_NUMBER;
        return kIOReturnSuccess;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_initialize:
      if (arguments->scalarInputCount > 0) {
        auto keyboardCountryCode = static_cast<uint32_t>(arguments->scalarInput[0]);
        ivars->provider->virtualHIDKeyboardInitialize(keyboardCountryCode);
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_ready:
      if (arguments->scalarOutput && arguments->scalarOutputCount > 0) {
        arguments->scalarOutput[0] = ivars->provider->virtualHIDKeyboardReady();
        return kIOReturnSuccess;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report: {
      IOMemoryDescriptor* memory = nullptr;

      auto kr = createIOMemoryDescriptor(arguments, &memory);
      if (kr == kIOReturnSuccess) {
        kr = ivars->provider->virtualHIDKeyboardPostReport(memory);
        OSSafeReleaseNULL(memory);
      }

      return kr;
    }

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_reset:
      return ivars->provider->virtualHIDKeyboardReset();

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_initialize:
      return ivars->provider->virtualHIDPointingInitialize();

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_ready:
      if (arguments->scalarOutput && arguments->scalarOutputCount > 0) {
        arguments->scalarOutput[0] = ivars->provider->virtualHIDPointingReady();
        return kIOReturnSuccess;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_post_report: {
      IOMemoryDescriptor* memory = nullptr;

      auto kr = createIOMemoryDescriptor(arguments, &memory);
      if (kr == kIOReturnSuccess) {
        kr = ivars->provider->virtualHIDPointingPostReport(memory);
        OSSafeReleaseNULL(memory);
      }

      return kr;
    }

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_reset:
      return ivars->provider->virtualHIDPointingReset();

    default:
      break;
  }

  return kIOReturnBadArgument;
}
