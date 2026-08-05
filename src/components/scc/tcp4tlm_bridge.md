# tcp4tlm_bridge Functionality

`tcp4tlm_bridge` is a SystemC/TLM component that tunnels loosely timed TLM
transactions over a TCP-based tcp4tlm connection. It is implemented in
`tcp4tlm_bridge.h` and `tcp4tlm_bridge.cpp`.

The bridge acts in two directions at once:

- As a local TLM target through `tsckt`, it receives local TLM transactions and
  serializes them into tcp4tlm FlatBuffer messages for a remote peer.
- As a local TLM initiator through `isckt`, it receives remote tcp4tlm messages,
  reconstructs TLM generic payloads, and forwards them into the local SystemC
  model.

Two bridge instances can therefore connect two SystemC simulations or component
hierarchies across a TCP connection.

## Main Interfaces

The `tcp4tlm_bridge` class derives from:

```cpp
sc_core::sc_module
tcp4tlm::server<tcp4tlm::request_message, tcp4tlm::response_message>
tcp4tlm::client<tcp4tlm::request_message, tcp4tlm::response_message>
```

It exposes the following main interfaces:

- `tsckt`: a TLM target socket for incoming local transactions.
- `isckt`: a TLM initiator socket for executing remote transactions locally.
- `signals`: a vector of `sc_out<bool>` ports for remote signal updates.
- TCP server functionality for receiving peer requests.
- TCP client functionality for sending requests to the peer.

The most important configuration parameters are:

```cpp
is_connection_server
other_host_name
other_host_port
this_host_port
wall_time_simulation_speed
write_no_response
no_systemc_sync
```

`wall_time_simulation_speed` initializes the shared wall-time speed limiter
during elaboration and changes how received tasks are scheduled. In this bridge
implementation, `no_systemc_sync` only disables periodic outgoing `SyncMsg`
messages on the non-server side.

## Connection Setup

During `before_end_of_elaboration()`, the bridge copies `other_host_name` and
`other_host_port` into the TCP client base class.

During `start_of_simulation()`, connection setup depends on
`is_connection_server`:

- If `is_connection_server` is `true`, the bridge starts a local server on
  `this_host_port` and waits for the peer to announce its endpoint.
- If `is_connection_server` is `false`, the bridge connects to the configured
  remote endpoint, starts its own server, sends a `NotifyEndpointMsg` containing
  its local listening endpoint, and waits for an OK response.

When a bridge receives `NotifyEndpointMsg`, it stores the peer host and port,
creates the reverse clientconnection, queues an OK response in SystemC context, 
and marks the connection as established.

This handshake creates a bidirectional link: each bridge has a server side for
receiving requests and a client side for sending requests.

## Forwarding Local TLM Transactions

Local TLM transactions enter the bridge through:

```cpp
btransport_cb()
transport_dbg_cb()
```

Both callbacks delegate to `do_access()`.

For a read transaction, `do_access()`:

1. Creates a `BusOpMsg` containing the current SystemC timestamp, delay,
   address, data length, access type, and copied byte-enable data.
2. Sends the message through the client connection.
3. Waits for a `BusDataMsg` response.
4. Verifies that the response status is OK and that the response belongs to the
   original request.
5. Copies the returned data into the original TLM payload.
6. Sets the TLM response status to `TLM_OK_RESPONSE`.

For a write transaction, `do_access()`:

1. Creates a `BusOpMsg` containing the current SystemC timestamp, delay,
   address, data length, access type, payload data and byte-enable data.
2. Sends the message through the client connection.
3. If `write_no_response` is enabled, marks the transaction as successful after
   sending.
4. Otherwise waits for a response and sets the TLM response status according to
   the received status.

Debug transport is represented on the wire as a `BusOpMsg` with debug access
type. On the receiving side the implementation reconstructs a normal
generic payload and executes it with `transport_dbg()`.

## Handling Remote Messages

Incoming tcp4tlm request messages are handled by `server_receive_completed()`.

The bridge supports these request payloads:

- `NotifyEndpointMsg`: announces the peer endpoint and completes the connection
  handshake.
- `BusOpMsg`: describes a remote TLM bus operation to execute locally.
- `SyncMsg`: carries a peer SystemC timestamp for synchronization.
- `SigOpMsg`: updates one of the bridge's boolean output signals.
- `NotifyShutdownMsg`: requests connection shutdown.

Unhandled messages are logged and answered with a declined response.

## Replaying Remote Bus Operations Locally

When `server_receive_completed()` receives a `BusOpMsg`, it:

1. Converts the message into a `tlm_generic_payload` using `init_gp()`.
2. Builds a SystemC time point from the message timestamp.
3. Places a callback task into the asynchronous task queue.
4. Pushes the timestamp into `next_time_stamp` when wall-time mode is disabled.
5. Waits until the queued task has completed.
6. Executes the transaction through either `isckt->b_transport(*gp, delay)` or `isckt->transport_dbr(*gp)`.
7. Sends a response back to the peer.

`init_gp()` sets the address, size, streaming width, command, response status,
and data buffer. A `BusOpMsg` without data is treated as a read; a message with
data is treated as a write and its data is copied into the payload buffer. If 
the `BusOpMsg` carries byte-enable data, the implementation applies incoming 
byte-enable data to the reconstructed generic payload.

Read requests return `BusDataMsg` with the data read from the local target.
Write requests return an OK or failure response unless the message requested no
response.

## Time Synchronization

The bridge starts three SystemC threads in its constructor:

```cpp
timing_thread()
process_task_que()
process_timed_task_que()
```

`timing_thread()` coordinates simulation time between peers:

- On the non-server side, it periodically sends `SyncMsg` messages containing
  the current SystemC timestamp every millisecond, unless `no_systemc_sync` is
  enabled.
- On the server side, after the peer connection is established, simulated-time
  mode consumes timestamps from `next_time_stamp` and advances local SystemC time
  toward them.
- On the server side with `wall_time_simulation_speed` enabled, the simulated
  timestamp advancement loop is skipped. Received tasks are executed immediately
  by `process_task_que()`.

`process_task_que()` moves received network tasks into SystemC execution
context. If `wall_time_simulation_speed` is enabled, or if the requested time
point has already passed, tasks run immediately. Otherwise they are scheduled
through the payload event queue relative to the current SystemC time.

`process_timed_task_que()` executes tasks when their scheduled SystemC event
fires.

## Signal Messages

`SigOpMsg` updates entries in the `signals` vector. If the message index is
valid, the bridge schedules a SystemC task at zero time, waits for it to write
the requested boolean value, and then sends an OK response. If the index is out
of range, it sends a declined response.

## Shutdown

`end_connection()` sends a final `SyncMsg` followed by `NotifyShutdownMsg` when
a remote connection is active.

When `NotifyShutdownMsg` is received, the bridge:

1. Closes the client socket.
2. Marks the client connection as disconnected.
3. Requests server shutdown if the server is still running.
4. Notifies `shutdown_evt` inside SystemC context.

The destructor and `end_of_simulation()` both ensure that the server is shut
down or requested to shut down and that an orderly shutdown message is sent when
appropriate. `end_of_simulation()` sends the shutdown message from the
non-server side when it is connected, then requests server shutdown; the
destructor sends it if needed and shuts the server down directly.

## Summary

`tcp4tlm_bridge` is a bidirectional TLM-over-TCP adapter. Local TLM operations
are serialized into tcp4tlm messages and sent to a peer, while remote messages
are converted back into local TLM transactions and executed through the bridge's
initiator socket. The implementation also manages endpoint negotiation,
response matching, optional write acknowledgments, simulation-time
synchronization, signal propagation, and orderly shutdown.
