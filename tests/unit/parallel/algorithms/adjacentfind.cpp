//  Copyright (c) 2014 Grant Mercer
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/parallel_adjacent_find.hpp>
#include <hpx/util/lightweight_test.hpp>
#include <boost/range/functions.hpp>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    // fill vector with random values about 1
    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), (std::rand() % 100) + 2);

    std::size_t random_pos = (std::rand() % 10005) + 2; //-V101

    c[random_pos] = 1;
    c[random_pos + 1] = 1;

    iterator index = hpx::parallel::adjacent_find(policy,
        iterator(boost::begin(c)), iterator(boost::end(c)));

    base_iterator test_index = boost::begin(c) + random_pos;

    HPX_TEST(index == iterator(test_index));
}

template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    // fill vector with random values above 1
    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), (std::rand() % 100) + 2);

    std::size_t random_pos = (std::rand() % 10005) + 2; //-V101

    c[random_pos] = 1;
    c[random_pos + 1] = 1;

    hpx::future<iterator> f =
        hpx::parallel::adjacent_find(p,
            iterator(boost::begin(c)), iterator(boost::end(c)));
    f.wait();

    // create iterator at position of value to be found
    base_iterator test_index = boost::begin(c) + random_pos;

    HPX_TEST(f.get() == iterator(test_index));
}

template <typename IteratorTag>
void test_adjacent_find()
{
    using namespace hpx::parallel;
    test_adjacent_find(seq, IteratorTag());
    test_adjacent_find(par, IteratorTag());
    test_adjacent_find(par_vec, IteratorTag());

    test_adjacent_find_async(seq(task), IteratorTag());
    test_adjacent_find_async(par(task), IteratorTag());

    test_adjacent_find(execution_policy(seq), IteratorTag());
    test_adjacent_find(execution_policy(par), IteratorTag());
    test_adjacent_find(execution_policy(par_vec), IteratorTag());

    test_adjacent_find(execution_policy(seq(task)), IteratorTag());
    test_adjacent_find(execution_policy(par(task)), IteratorTag());
}

void adjacent_find_test()
{
    test_adjacent_find<std::random_access_iterator_tag>();
    test_adjacent_find<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find_exception(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator <base_iterator, IteratorTag>
        decorated_iterator;
    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand() + 1);

    bool caught_exception = false;
    try {
        hpx::parallel::adjacent_find(policy,
            decorated_iterator(
                boost::begin(c),
                [](){ throw std::runtime_error("test"); }),
            decorated_iterator(boost::end(c)));
        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e) {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch (...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find_exception_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator <base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand() + 1);

    bool caught_exception = false;
    bool returned_from_algorithm = false;

    try {
        hpx::future<decorated_iterator> f =
            hpx::parallel::adjacent_find(p,
                decorated_iterator(
                    boost::begin(c),
                    [](){ throw std::runtime_error("test"); }),
                decorated_iterator(boost::end(c)));

        returned_from_algorithm = true;

        f.get();

        HPX_TEST(false);
    }
    catch(hpx::exception_list const& e) {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch (...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_adjacent_find_exception()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_adjacent_find_exception(seq, IteratorTag());
    test_adjacent_find_exception(par, IteratorTag());

    test_adjacent_find_exception_async(seq(task), IteratorTag());
    test_adjacent_find_exception_async(par(task), IteratorTag());

    test_adjacent_find_exception(execution_policy(seq), IteratorTag());
    test_adjacent_find_exception(execution_policy(par), IteratorTag());

    test_adjacent_find_exception(execution_policy(seq(task)), IteratorTag());
    test_adjacent_find_exception(execution_policy(par(task)), IteratorTag());
}

void adjacent_find_exception_test()
{
    test_adjacent_find_exception<std::random_access_iterator_tag>();
    test_adjacent_find_exception<std::forward_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find_bad_alloc(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator <base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand() + 1);

    bool caught_bad_alloc = false;
    try {
        hpx::parallel::adjacent_find(policy,
            decorated_iterator(
                boost::begin(c),
                [](){ throw std::bad_alloc(); }),
            decorated_iterator(boost::end(c)));
        HPX_TEST(false);
    }
    catch (std::bad_alloc const&) {
        caught_bad_alloc = true;
    }
    catch (...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_adjacent_find_bad_alloc_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator <base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand() + 1);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;

    try {
        hpx::future<decorated_iterator> f =
            hpx::parallel::adjacent_find(p,
            decorated_iterator(
                boost::begin(c),
                [](){ throw std::bad_alloc(); }),
            decorated_iterator(boost::end(c)));

        returned_from_algorithm = true;

        f.get();

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&) {
        caught_bad_alloc = true;
    }
    catch (...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_adjacent_find_bad_alloc()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_adjacent_find_bad_alloc(seq, IteratorTag());
    test_adjacent_find_bad_alloc(par, IteratorTag());

    test_adjacent_find_bad_alloc_async(seq(task), IteratorTag());
    test_adjacent_find_bad_alloc_async(par(task), IteratorTag());

    test_adjacent_find_bad_alloc(execution_policy(seq), IteratorTag());
    test_adjacent_find_bad_alloc(execution_policy(par), IteratorTag());

    test_adjacent_find_bad_alloc(execution_policy(seq(task)), IteratorTag());
    test_adjacent_find_bad_alloc(execution_policy(par(task)), IteratorTag());
}

void adjacent_find_bad_alloc_test()
{
    test_adjacent_find_bad_alloc<std::random_access_iterator_tag>();
    test_adjacent_find_bad_alloc<std::forward_iterator_tag>();
}

int hpx_main(boost::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int)std::time(0);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    adjacent_find_test();
    adjacent_find_exception_test();
    adjacent_find_bad_alloc_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace boost::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run")
        ;

    // By default this test should run on all available cores
    std::vector<std::string> cfg;
    cfg.push_back("hpx.os_threads=" +
        boost::lexical_cast<std::string>(hpx::threads::hardware_concurrency()));

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
