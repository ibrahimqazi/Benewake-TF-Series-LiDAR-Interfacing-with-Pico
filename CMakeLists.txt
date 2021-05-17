add_executable(tf_series tf_series.c)

# Pull in our pico_stdlib which pulls in commonly used features, also add hardware uart because we are going to use uart port
target_link_libraries(tf_series pico_stdlib hardware_uart)

# enable/diable usb output, and uart output
pico_enable_stdio_usb(tf_series 1) # 1 means enable and 0 means disable
pico_enable_stdio_uart(tf_series 1)


# create map/bin/hex file etc.
pico_add_extra_outputs(tf_series)

# add url via pico_set_program_url
example_auto_set_url(tf_series)

