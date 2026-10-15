#include "common.hpp"

#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>

// #region capture_exception
void background_work(std::exception_ptr& eptr)
{
    try
    {
        may_throw(true);

        // ...
    }
    catch (...)
    {
        eptr = std::current_exception();
    }
}
// #endregion

// #region handle_exception
void run_and_handle_exception()
{
    std::exception_ptr thd_exception;

    std::thread thd(&background_work, std::ref(thd_exception));
    thd.join();

    if (thd_exception)
    {
        try
        {
            std::rethrow_exception(thd_exception);
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Caught: " << e.what() << std::endl;
        }
    }
}
// #endregion

int main()
{
    run_and_handle_exception();
}
