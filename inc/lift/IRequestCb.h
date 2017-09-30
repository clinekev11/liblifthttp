#pragma once

namespace lift
{

/**
 * Interface for the set of possible asynchronous request callbacks when
 * the request completes/times out/errors.
 */
class IRequestCb
{
public:
    virtual ~IRequestCb() = default;

    /**
     * Request callback on completion.  The request will have either a success
     * status or an error set.
     * @param request The request that has completed.
     */
    virtual auto OnComplete(
        std::unique_ptr<AsyncRequest> request
    ) -> void = 0;
};

} // lift