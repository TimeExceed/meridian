# Meridian

Meridian is a framework of nano-service based on Lua-5.3 and C++-11.
The difference between meridian and micro-service is that usually instances of micro-services are isolated by processes meanwhile those of meridian nano-services by lua VMs.

# Concepts & Terminology

### (nano-)service & instance

A (nano-)service is a couple of code which provides one particular functionality.
An instance of a service is a run of the service.

A service always has a unique provider-specified string name.
In order to comparing them efficiently, we prefer a dot-separated flavour with concrete part first, e.g., alice.mankind.animal.

An instance has a process-wide unique integer id.
Usually a instance is running in a lua vm.
Instance id 0 is reserved.

There are 3 types of services, classified by life-cycle of their instances.

*   one-shot

    An one-shot instance serves exactly one request.

*   eternal

    An eternal instance will run as long as the service runs.
    An eternal service usually has a fixed number of instances, and each instance serves an infinite series of requests (usually by a queue).

*   long-lived

    A long-lived instance usually serves a stream of requests, e.g., a TCP connection.
    These instances are going to die when the underlying stream finishes.
   
### request

A request is something to do.
It has a process-wide unique integer id (0 is reserved), an optional respond-to request id, a serving instance id and a series of parameters for serving this request.

### scheduler

The one who attaches suitable instances to physical threads.
It is also a delegate between instances and instances, instances and services.

# API

## Service API

### for the scheduler (C++)

*   `start()` -> nil

*   `newInstance()` -> a new lua vm

    return a new lua vm filled a function.
    provided by one-shot service and long-lived service.

*   `getName()` -> a std::string

### for instances, delegated by scheduler (lua)

*   `newInstance(service)` -> instance

    provided by one-shot services.

*   `getInstance(service, key)` -> instance

    return the instance id associated with a key.
    if there is no instance associated with the key, create one.
    provided by long-lived services.

*   `getInstanceList(service name)` -> [instance]

    provided by eternal services only.

## Instance API

### for scheduler (lua)

*   `close()` -> nil

    do some cleanup

### for other instance, delegated by scheduler (lua)

*   `run(opts, ...)` -> nil/[request]

    serve a request.
    when it finishes, it returns.
    when it keeps alive, it yields.
    opts is a lua table containing id of this request and an optional id of respond-to request.

## Scheduler API

### initialization (C++)

*   `startThreads(int)` -> nil
*   `startServices([service object])` -> nil

### for instances (lua extension)

*   `newInstance(service)` -> instance
*   `getInstance(service, key)` -> instance
*   `getInstanceList(service)` -> [instance]
*   `issueRequestAndRespondToMe(serving instance, args)` -> request
*   `issueRequestAndRespondTo(respond-to request, serving instance, args)` -> nil
*   `issueRequest(serving instance, args)` -> nil

### for instances (C++, implicitly called by yield)

*   `waitForAny([request])` -> request

    wait for any of [request] is responded.
    Especially, request 0 is the incoming request.

## Core Service

### timeout
