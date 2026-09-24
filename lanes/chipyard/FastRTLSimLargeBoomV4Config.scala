// Add to generators/chipyard/src/main/scala/config/BoomConfigs.scala at Chipyard 1.13.0.
// Large BOOM v4 (3-wide, 6 integer RF read ports) without TileLink monitors for fast RTL simulation.
class FastRTLSimLargeBoomV4Config extends Config(
  new freechips.rocketchip.subsystem.WithoutTLMonitors ++
  new boom.v4.common.WithNLargeBooms(1) ++
  new chipyard.config.WithSystemBusWidth(128) ++
  new chipyard.config.AbstractConfig)
