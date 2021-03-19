#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard.h"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDPointing.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp"
#include "version.hpp"
#include <HIDDriverKit/IOHIDDeviceKeys.h>
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define LOG_PREFIX "Karabiner-DriverKit-VirtualHIDDeviceRoot " KARABINER_DRIVERKIT_VERSION

struct org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot_IVars {
  uint32_t keyboardCountryCode;
  org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard* keyboard;
  org_pqrs_Karabiner_DriverKit_VirtualHIDPointing* pointing;
};

bool org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  OSSafeReleaseNULL(ivars->keyboard);
  OSSafeReleaseNULL(ivars->pointing);

  IOSafeDeleteNULL(ivars, org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot_IVars, 1);

  super::free();
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, Start) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start");

  auto kr = Start(provider, SUPERDISPATCH);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " super::Start failed: 0x%x", kr);
    Stop(provider, SUPERDISPATCH);
    return kr;
  }

  RegisterService();

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  if (ivars->keyboard) {
    ivars->keyboard->Stop(provider);
  }
  if (ivars->pointing) {
    ivars->pointing->Stop(provider);
  }

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, NewUserClient) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " NewUserClient type:%d", type);

  IOService* client;

  auto kr = Create(this, "UserClientProperties", &client);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
    return kr;
  }

  os_log(OS_LOG_DEFAULT, LOG_PREFIX " UserClient is created");

  *userClient = OSDynamicCast(IOUserClient, client);
  if (!*userClient) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
    client->release();
    return kIOReturnError;
  }

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, virtualHIDKeyboardInitialize) {
  if (!ivars->keyboard) {
    ivars->keyboardCountryCode = keyboardCountryCode;

    IOService* client;

    auto kr = Create(this, "VirtualHIDKeyboardProperties", &client);
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
      return kr;
    }

    ivars->keyboard = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, client);
    if (!ivars->keyboard) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
      client->release();
      return kIOReturnError;
    }
  }

  return kIOReturnSuccess;
}

bool IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, virtualHIDKeyboardReady) {
  return (ivars->keyboard && ivars->keyboard->getReady());
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, virtualHIDKeyboardPostReport) {
  if (ivars->keyboard) {
    return ivars->keyboard->postReport(report);
  }

  return kIOReturnError;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, virtualHIDKeyboardReset) {
  if (ivars->keyboard) {
    return ivars->keyboard->reset();
  }
  return kIOReturnError;
}
