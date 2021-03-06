# Copyright (c) 2007-2013 Hartmut Kaiser
# Copyright (c) 2014-2015 Thomas Heller
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(HPX_AddLibrary)

if(HPX_WITH_PARCELPORT_IPC)
  if(NOT Boost_VERSION GREATER 105100)
    hpx_error("HPX_WITH_PARCELPORT_IPC=On is on but the ipc parcelport only works with at least Boost 1.52")
  endif()
  hpx_add_config_define(HPX_HAVE_PARCELPORT_IPC)

  macro(add_parcelport_ipc_module)
    hpx_debug("add_parcelport_ipc_module")
    add_parcelport(ipc
        SOURCES "${hpx_SOURCE_DIR}/plugins/parcelport/ipc/connection_handler_ipc.cpp"
                "${hpx_SOURCE_DIR}/plugins/parcelport/ipc/parcelport_ipc.cpp"
        HEADERS
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/acceptor.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/connection_handler.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/data_buffer_cache.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/data_buffer.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/data_window.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/interprocess_errors.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/locality.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/message.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/receiver.hpp"
              "${hpx_SOURCE_DIR}/hpx/plugins/parcelport/ipc/sender.hpp"
        FOLDER "Core/Plugins/Parcelport/Tcp"
        )
  endmacro()
else()
  macro(add_parcelport_ipc_module)
  endmacro()
endif()
