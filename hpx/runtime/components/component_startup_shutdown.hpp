//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_COMPONENT_STARTUP_SHUTDOWN_SEP_20_2011_0217PM)
#define HPX_COMPONENT_STARTUP_SHUTDOWN_SEP_20_2011_0217PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/runtime/components/component_startup_shutdown_base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace components
{
    ///////////////////////////////////////////////////////////////////////////
    /// The \a component_startup_shutdown class provides a minimal
    /// implementation of a component's startup/shutdown function provider.
    template <bool(*Startup)(util::function_nonser<void()>&, bool&),
        bool(*Shutdown)(util::function_nonser<void()>&, bool&)>
    struct component_startup_shutdown : public component_startup_shutdown_base
    {
        ///
        ~component_startup_shutdown() {}

        /// \brief Return any startup function for this component
        ///
        /// \param startup  [in, out] The module is expected to fill this
        ///                 function object with a reference to a startup
        ///                 function. This function will be executed by the
        ///                 runtime system during system startup.
        ///
        /// \return Returns \a true if the parameter \a startup has been
        ///         successfully initialized with the startup function.
        bool get_startup_function(util::function_nonser<void()>& startup_,
            bool& pre_startup_)
        {
            return Startup(startup_, pre_startup_);
        }

        /// \brief Return any startup function for this component
        ///
        /// \param shutdown [in, out] The module is expected to fill this
        ///                 function object with a reference to a startup
        ///                 function. This function will be executed by the
        ///                 runtime system during system startup.
        ///
        /// \return Returns \a true if the parameter \a shutdown has been
        ///         successfully initialized with the shutdown function.
        bool get_shutdown_function(util::function_nonser<void()>& shutdown_,
            bool& pre_shutdown_)
        {
            return Shutdown(shutdown_, pre_shutdown_);
        }
    };
}}

///////////////////////////////////////////////////////////////////////////////
#define HPX_DEFINE_COMPONENT_STARTUP_SHUTDOWN(startup_, shutdown_)            \
    namespace hpx { namespace components { namespace startup_shutdown_provider\
    {                                                                         \
        bool BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _startup)(             \
             util::function_nonser<void()>& startup_func, bool& pre_startup)  \
        {                                                                     \
            util::function_nonser<bool(util::function_nonser<void()>&, bool&)> tmp =\
                static_cast<bool(*)(util::function_nonser<void()>&, bool&)>(startup_);\
            if (!!tmp) { return tmp(startup_func, pre_startup); }             \
            return false;                                                     \
        }                                                                     \
        bool BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _shutdown)(            \
             util::function_nonser<void()>& shutdown_func, bool& pre_shutdown)\
        {                                                                     \
            util::function_nonser<bool(util::function_nonser<void()>&, bool&)> tmp =\
                static_cast<bool(*)(util::function_nonser<void()>&, bool&)>(shutdown_);\
            if (!!tmp) { return tmp(shutdown_func, pre_shutdown); }           \
            return false;                                                     \
        }                                                                     \
    }}}                                                                       \
    /***/

///////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(startup, shutdown)              \
    HPX_DEFINE_COMPONENT_STARTUP_SHUTDOWN(startup, shutdown)                  \
    namespace hpx { namespace components { namespace startup_shutdown_provider\
    {                                                                         \
        typedef component_startup_shutdown<                                   \
            BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _startup),              \
            BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _shutdown)              \
        > BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider);               \
    }}}                                                                       \
    namespace hpx { namespace components                                      \
    {                                                                         \
        template struct component_startup_shutdown<                           \
            startup_shutdown_provider::                                       \
                BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _startup),          \
            startup_shutdown_provider::                                       \
                BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _shutdown)          \
        >;                                                                    \
    }}                                                                        \
    /**/

#define HPX_REGISTER_STARTUP_SHUTDOWN_MODULE(startup, shutdown)               \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS()                                 \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(startup, shutdown)                  \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY(                                   \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/
#define HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_DYNAMIC(startup, shutdown)       \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS_DYNAMIC()                         \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(startup, shutdown)                  \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY_DYNAMIC(                           \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/

#define HPX_REGISTER_STARTUP_MODULE(startup)                                  \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS()                                 \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(startup, 0)                         \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY(                                   \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/
#define HPX_REGISTER_STARTUP_MODULE_DYNAMIC(startup)                          \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS_DYNAMIC()                         \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(startup, 0)                         \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY_DYNAMIC(                           \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/

#define HPX_REGISTER_SHUTDOWN_MODULE(shutdown)                                \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS()                                 \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(0, shutdown)                        \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY(                                   \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/
#define HPX_REGISTER_SHUTDOWN_MODULE_DYNAMIC(shutdown)                        \
    HPX_REGISTER_STARTUP_SHUTDOWN_FUNCTIONS_DYNAMIC()                         \
    HPX_REGISTER_STARTUP_SHUTDOWN_MODULE_(0, shutdown)                        \
    HPX_REGISTER_STARTUP_SHUTDOWN_REGISTRY_DYNAMIC(                           \
        hpx::components::startup_shutdown_provider::                          \
        BOOST_PP_CAT(HPX_PLUGIN_COMPONENT_PREFIX, _provider), startup_shutdown) \
    /**/

#endif // HPX_A7F46A4F_9AF9_4909_B0D8_5304FEFC5649

