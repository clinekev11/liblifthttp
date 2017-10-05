#include <lift/Lift.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

class CompletedCtx : public lift::IRequestCb
{
public:
    CompletedCtx(
        size_t total_requests
    )
        : m_completed(total_requests)
    {

    }

    std::atomic<size_t> m_completed;    ///< This variable signals to the main thread all the requests are completed.

    auto OnComplete(lift::Request request) -> void override
    {
        m_completed--;
        switch(request->GetStatus())
        {
            case lift::RequestStatus::SUCCESS:
                std::cout
                    << "Completed " << request->GetUrl()
                    << " ms:" << request->GetTotalTimeMilliseconds() << std::endl;
                break;
            case lift::RequestStatus::CONNECT_ERROR:
                std::cout << "Unable to connect to: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::CONNECT_DNS_ERROR:
                std::cout << "Unable to lookup DNS entry for: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::CONNECT_SSL_ERROR:
                std::cout << "SSL Error for: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::TIMEOUT:
                std::cout << "Timeout: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::RESPONSE_EMPTY:
                std::cout << "No response received: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::ERROR:
                std::cout << "Request had an unrecoverable error: " << request->GetUrl() << std::endl;
                break;
            case lift::RequestStatus::BUILDING:
            case lift::RequestStatus::EXECUTING:
                std::cout
                    << "Request is in an invalid state: "
                    << request_status2str(request->GetStatus()) << std::endl;
                break;
        }
    }
};

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::RequestPool request_pool;
    std::vector<std::string> urls =
    {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.reddit.com"
    };

    // Create the EventLoop with a Request callback 'Completed'.
    lift::EventLoop event_loop(std::make_unique<CompletedCtx>(urls.size()));

    {
        std::vector<lift::Request> requests;
        requests.reserve(urls.size());
        for(auto& url : urls) {
            requests.emplace_back(request_pool.Produce(url, 250ms));
        }

        /**
         * This will 'move' all of the Request objects into the event loop.
         * The values in the 'requests' vector are now no longer valid.  This
         * example intentionally has 'requests' go out of scope to further
         * demonstrate this.
         */
        event_loop.AddRequests(requests);
    }

    // Now wait for all the requests to finish before cleaning up.
    const auto& completed_ctx = static_cast<CompletedCtx&>(event_loop.GetRequestCallback());
    while(completed_ctx.m_completed > 0)
    {
        std::this_thread::sleep_for(100ms);
    }

    // Cleanup EventLoop / Threads and LiftHttp library.
    event_loop.Stop();

    lift::cleanup();

    return 0;
}