ADD_SUBDIRECTORY ( ambix_encoder )

# Legacy ambix_encoder_i{2,4,6,8} variants were merged into the single
# ambix_encoder binary in v0.5. The number of active source inputs is now
# selectable at runtime (1..MAX_INPUT_CHANNELS, default 8). Project sessions
# referencing the old plugin codes (AE2U, AE4U, AE6U, AE8U) need to be
# re-instantiated with ambix_encoder; per-session positions can be re-entered
# either via the per-source inspector (unlinked mode) or the linked
# azimuth/width controls.
