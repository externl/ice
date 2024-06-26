//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `ConnectionInfo.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

import Foundation

/// Provides access to the connection details of an IAP connection.
public protocol IAPConnectionInfo: ConnectionInfo {
  /// The accessory name.
  var name: Swift.String { get set }
  /// The accessory manufacturer.
  var manufacturer: Swift.String { get set }
  /// The accessory model number.
  var modelNumber: Swift.String { get set }
  /// The accessory firmare revision.
  var firmwareRevision: Swift.String { get set }
  /// The accessory hardware revision.
  var hardwareRevision: Swift.String { get set }
  /// The protocol used by the accessory.
  var `protocol`: Swift.String { get set }
}
