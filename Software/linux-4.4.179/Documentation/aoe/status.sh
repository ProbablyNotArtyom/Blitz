Operating FCoE using bnx2fc
===========================
Broadcom FCoE offload through bnx2fc is full stateful hardware offload that
cooperates with all interfaces provided by the Linux ecosystem for FC/FCoE and
SCSI controllers.  As such, FCoE functionality, once enabled is largely
transparent. Devices discovered on the SAN will be registered and unregistered
automatically with the upper storage layers.

Despite the fact that the Broadcom's FCoE offload is fully offloaded, it does
depend on the state of the network interfaces to operate. As such, the network
interface (e.g. eth0) associated with the FCoE offload initiator must be 'up'.
It is recommended that the network interfaces be c