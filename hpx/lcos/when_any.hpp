//  Copyright (c) 2007-2015 Hartmut Kaiser
//  Copyright (c) 2013 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file lcos/when_any.hpp

#if !defined(HPX_LCOS_WHEN_ANY_APR_17_2012_1143AM)
#define HPX_LCOS_WHEN_ANY_APR_17_2012_1143AM

#if defined(DOXYGEN)
namespace hpx
{
    ///////////////////////////////////////////////////////////////////////////
    /// Result type for \a when_any, contains a sequence of futures and an
    /// index pointing to a ready future.
    template <typename Sequence>
    struct when_any_result
    {
        std::size_t index;  ///< The index of a future which has become ready
        Sequence futures;   ///< The sequence of futures as passed to \a hpx::when_any
    };

    /// The function \a when_any is a non-deterministic choice operator. It
    /// OR-composes all future objects given and returns a new future object
    /// representing the same list of futures after one future of that list
    /// finishes execution.
    ///
    /// \param first    [in] The iterator pointing to the first element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_any should wait.
    /// \param last     [in] The iterator pointing to the last element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_any should wait.
    ///
    /// \return   Returns a when_any_result holding the same list of futures
    ///           as has been passed to when_any and an index pointing to a
    ///           ready future.
    ///           - future<when_any_result<vector<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output vector will be the same as given by the input
    ///             iterator.
    template <typename InputIter>
    future<when_any_result<
        vector<future<typename std::iterator_traits<InputIter>::value_type>>>>
    when_any(InputIter first, InputIter last);

    /// The function \a when_any is a non-deterministic choice operator. It
    /// OR-composes all future objects given and returns a new future object
    /// representing the same list of futures after one future of that list
    /// finishes execution.
    ///
    /// \param futures  [in] A vector holding an arbitrary amount of \a future or
    ///                 \a shared_future objects for which \a when_any should
    ///                 wait.
    ///
    /// \return   Returns a when_any_result holding the same list of futures
    ///           as has been passed to when_any and an index pointing to a
    ///           ready future.
    ///           - future<when_any_result<vector<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output vector will be the same as given by the input
    ///             iterator.
    template <typename R>
    future<when_any_result<
        std::vector<future<R>>>>
    when_any(std::vector<future<R>>& futures);

    /// The function \a when_any is a non-deterministic choice operator. It
    /// OR-composes all future objects given and returns a new future object
    /// representing the same list of futures after one future of that list
    /// finishes execution.
    ///
    /// \param futures  [in] An arbitrary number of \a future or \a shared_future
    ///                 objects, possibly holding different types for which
    ///                 \a when_any should wait.
    ///
    /// \return   Returns a when_any_result holding the same list of futures
    ///           as has been passed to when_any and an index pointing to a
    ///           ready future..
    ///           - future<when_any_result<tuple<future<T0>, future<T1>...>>>:
    ///             If inputs are fixed in number and are of heterogeneous
    ///             types. The inputs can be any arbitrary number of future
    ///             objects.
    ///           - future<when_any_result<tuple<>>> if \a when_any is called
    ///             with zero arguments.
    ///             The returned future will be initially ready.
    template <typename ...T>
    future<when_any_result<tuple<future<T>...>>>
    when_any(T &&... futures);

    /// The function \a when_any_n is a non-deterministic choice operator. It
    /// OR-composes all future objects given and returns a new future object
    /// representing the same list of futures after one future of that list
    /// finishes execution.
    ///
    /// \param first    [in] The iterator pointing to the first element of a
    ///                 sequence of \a future or \a shared_future objects for
    ///                 which \a when_any_n should wait.
    /// \param count    [in] The number of elements in the sequence starting at
    ///                 \a first.
    ///
    /// \return   Returns a when_any_result holding the same list of futures
    ///           as has been passed to when_any and an index pointing to a
    ///           ready future.
    ///           - future<when_any_result<vector<future<R>>>>: If the input
    ///             cardinality is unknown at compile time and the futures
    ///             are all of the same type. The order of the futures in the
    ///             output vector will be the same as given by the input
    ///             iterator.
    ///
    /// \note     None of the futures in the input sequence are invalidated.
    template <typename InputIter>
    future<when_any_result<
        vector<future<typename std::iterator_traits<InputIter>::value_type>>>>
    when_any_n(InputIter first, std::size_t count);
}

#else // DOXYGEN

#include <hpx/hpx_fwd.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/lcos/when_any.hpp>
#include <hpx/lcos/local/packaged_task.hpp>
#include <hpx/lcos/local/packaged_continuation.hpp>
#include <hpx/runtime/threads/thread.hpp>
#include <hpx/util/always_void.hpp>
#include <hpx/util/bind.hpp>
#include <hpx/util/decay.hpp>
#include <hpx/util/move.hpp>
#include <hpx/util/tuple.hpp>
#include <hpx/traits/acquire_future.hpp>

#include <boost/atomic.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/utility/swap.hpp>

#include <algorithm>
#include <iterator>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace lcos
{
    template <typename Sequence>
    struct when_any_result
    {
        static std::size_t index_error()
        {
            return static_cast<std::size_t>(-1);
        }

        when_any_result()
          : index(static_cast<size_t>(index_error()))
          , futures()
        {}

        explicit when_any_result(Sequence&& futures)
          : index(index_error())
          , futures(std::move(futures))
        {}

        when_any_result(when_any_result const& rhs)
          : index(rhs.index), futures(rhs.futures)
        {}

        when_any_result(when_any_result&& rhs)
          : index(rhs.index), futures(std::move(rhs.futures))
        {
            rhs.index = index_error();
        }

        when_any_result& operator=(when_any_result const& rhs)
        {
            if (this != &rhs)
            {
                index = rhs.index;
                futures = rhs.futures;
            }
            return true;
        }

        when_any_result& operator=(when_any_result && rhs)
        {
            if (this != &rhs)
            {
                index = rhs.index;
                rhs.index = index_error();
                futures = std::move(rhs.futures);
            }
            return true;
        }

        std::size_t index;
        Sequence futures;
    };

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        template <typename Sequence>
        struct when_any_frame;

        template <typename Sequence>
        struct set_when_any_callback_impl
        {
            explicit set_when_any_callback_impl(when_any_frame<Sequence>& when)
              : when_(when), idx_(0)
            {}

            template <typename Future>
            void operator()(Future& future) const
            {
                std::size_t index = when_.index_.load(boost::memory_order_seq_cst);
                if (index == when_any_result<Sequence>::index_error()) {
                    if (!future.is_ready()) {
                        // handle future only if not enough futures are ready yet
                        // also, do not touch any futures which are already ready

                        typedef
                            typename lcos::detail::shared_state_ptr_for<Future>::type
                            shared_state_ptr;

                        boost::intrusive_ptr<when_any_frame<Sequence> > that_(&when_);
                        shared_state_ptr const& shared_state =
                            lcos::detail::get_shared_state(future);
                        shared_state->execute_deferred();
                        shared_state->set_on_completed(util::bind(
                            &when_any_frame<Sequence>::on_future_ready, std::move(that_),
                            idx_, threads::get_self_id()));
                    }
                    else {
                        if (when_.index_.compare_exchange_strong(index, idx_))
                        {
                            when_.goal_reached_on_calling_thread_ = true;
                        }
                    }
                }
                ++idx_;
            }

            template <typename Sequence_>
            void apply(Sequence_& sequence, typename boost::enable_if<
                boost::fusion::traits::is_sequence<Sequence_> >::type* = 0) const
            {
                boost::fusion::for_each(sequence, *this);
            }

            template <typename Sequence_>
            void apply(Sequence_& sequence, typename boost::disable_if<
                boost::fusion::traits::is_sequence<Sequence_> >::type* = 0) const
            {
                std::for_each(sequence.begin(), sequence.end(), *this);
            }

            when_any_frame<Sequence>& when_;
            mutable std::size_t idx_;
        };

        template <typename Sequence>
        void set_on_completed_callback(when_any_frame<Sequence>& when)
        {
            set_when_any_callback_impl<Sequence> callback(when);
            callback.apply(when.t_.futures);
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Sequence>
        struct when_any_frame //-V690
          : hpx::lcos::detail::future_data<when_any_result<Sequence> >
        {
            typedef when_any_result<Sequence> result_type;
            typedef hpx::lcos::future<result_type> type;

        private:
            // workaround gcc regression wrongly instantiating constructors
            when_any_frame();
            when_any_frame(when_any_frame const&);

        public:
            template <typename Tuple_>
            when_any_frame(Tuple_ && t)
              : t_(std::forward<Tuple_>(t))
              , index_(when_any_result<Sequence>::index_error())
              , goal_reached_on_calling_thread_(false)
            {}

            void on_future_ready(std::size_t idx, threads::thread_id_type const& id)
            {
                std::size_t index_not_initialized =
                    when_any_result<Sequence>::index_error();
                if (index_.compare_exchange_strong(index_not_initialized, idx))
                {
                    // reactivate waiting thread only if it's not us
                    if (id != threads::get_self_id())
                        threads::set_thread_state(id, threads::pending);
                    else
                        goal_reached_on_calling_thread_ = true;
                }
            }

            result_type operator()()
            {
                // set callback functions to executed when future is ready
                set_on_completed_callback(*this);

                // if one of the requested futures is already set, our
                // callback above has already been called often enough, otherwise
                // we suspend ourselves
                if (!goal_reached_on_calling_thread_)
                {
                    // wait for any of the futures to return to become ready
                    this_thread::suspend(threads::suspended,
                        "hpx::lcos::detail::when_any_frame::operator()");
                }

                // that should not happen
                HPX_ASSERT(index_.load() != when_any_result<Sequence>::index_error());

                t_.index = index_.load();
                return std::move(t_);
            }

            result_type t_;
            boost::atomic<std::size_t> index_;
            bool goal_reached_on_calling_thread_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Future>
    lcos::future<when_any_result<std::vector<Future> > >
    when_any(std::vector<Future>& values)
    {
        BOOST_STATIC_ASSERT_MSG(
            traits::is_future<Future>::value, "invalid use of when_any");

        typedef std::vector<Future> result_type;
        typedef detail::when_any_frame<result_type> frame_type;

        result_type values_;
        values_.reserve(values.size());
        std::transform(values.begin(), values.end(),
            std::back_inserter(values_),
            traits::acquire_future_disp());

        boost::intrusive_ptr<frame_type> f(new frame_type(std::move(values_)));
        lcos::local::futures_factory<when_any_result<result_type>()> p(
            util::bind(&detail::when_any_frame<result_type>::operator(), f));
        p.apply();

        return p.get_future();
    }

    template <typename Future>
    lcos::future<when_any_result<std::vector<Future> > > //-V659
    when_any(std::vector<Future> && t)
    {
        return lcos::when_any(t);
    }

    template <typename Iterator>
    lcos::future<when_any_result<std::vector<
        typename lcos::detail::future_iterator_traits<Iterator>::type
    > > >
    when_any(Iterator begin, Iterator end)
    {
        typedef
            typename lcos::detail::future_iterator_traits<Iterator>::type
            future_type;
        typedef std::vector<future_type> result_type;

        result_type values;
        std::transform(begin, end, std::back_inserter(values),
            traits::acquire_future_disp());

        return lcos::when_any(values);
    }

    inline lcos::future<when_any_result<hpx::util::tuple<> > > //-V524
    when_any()
    {
        typedef when_any_result<hpx::util::tuple<> > result_type;

        return lcos::make_ready_future(result_type());
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    lcos::future<when_any_result<std::vector<
        typename lcos::detail::future_iterator_traits<Iterator>::type
    > > >
    when_any_n(Iterator begin, std::size_t count)
    {
        typedef
            typename lcos::detail::future_iterator_traits<Iterator>::type
            future_type;
        typedef std::vector<future_type> result_type;

        result_type values;
        values.reserve(count);

        traits::acquire_future_disp func;
        for (std::size_t i = 0; i != count; ++i)
            values.push_back(func(*begin++));

        return lcos::when_any(values);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename... Ts>
    lcos::future<when_any_result<
        hpx::util::tuple<typename traits::acquire_future<Ts>::type...>
    > >
    when_any(Ts&&... ts)
    {
        typedef hpx::util::tuple<
                typename traits::acquire_future<Ts>::type...
            > result_type;
        typedef detail::when_any_frame<result_type> frame_type;

        traits::acquire_future_disp func;
        result_type values(func(std::forward<Ts>(ts))...);

        boost::intrusive_ptr<frame_type> f(new frame_type(std::move(values)));
        lcos::local::futures_factory<when_any_result<result_type>()> p(
            util::bind(&detail::when_any_frame<result_type>::operator(), f));
        p.apply();

        return p.get_future();
    }
}}

namespace hpx
{
    using lcos::when_any_result;
    using lcos::when_any;
    using lcos::when_any_n;
}

#endif // DOXYGEN
#endif
