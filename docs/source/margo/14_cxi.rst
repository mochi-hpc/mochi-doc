HPE Slingshot (CXI) network considerations
==========================================

HPE systems that use the Slingshot (CXI) network fabric require some special
considerations for use with Margo.  Examples are documented in the `platform-configurations repository <https://github.com/mochi-hpc-experiments/platform-configurations>`_.

The most important concept to understand is that in order for two processes
to communicate over the Slingshot network, then they must share a VNI
(virtual network interface).  A VNI is a hardware-enforced authentication
mechanism that is usually provisioned by the system's resource manager as
needed.  MPI is the most common user of VNIs; all processes in an MPI
application that use the Slingshot network also must share a common VNI.

There are four general types of VNIs that could be used by Mochi:

* job step level VNIs: these span all processes launched by a single `mpiexec`
  or `srun` invocation.  This type of VNI is most commonly used for MPI
  communication.
* job level VNIs: these VNIs span all processes within a given job
  allocation (whether they share a common `mpiexec`/`srun` invocation or not).
  This is the most common type of VNI to be used by Mochi, but it must be
  explicitly requested by the user.
* system level VNIs: these are system-wide default VNIs that enable
  communication across all processes in a system (regardless of what job
  they belong too). This type of VNI is disabled on most systems for
  improved security and isolation.  You can check if the default system VNI
  is enabled by running `cxi_service list -v -s 1` and inspecting the
  output.
* dynamic VNIs: these are VNIs that are allocated on demand by an external
  service (i.e., `drc2`) to support other configurations, such as
  communications that should span jobs.  At present this option is not
  available on any known systems that Mochi supports.

Using a job level VNI (most common scenario)
--------------------------------------------
This is the preferred method no most systems.  In order to use it, you must
launch all processes using either `srun` (on SLURM systems) or `mpiexec` (on
PBS Pro systems) and use the following options:

* for SLURM: `--network job_vni,single_node_vni`
* for PBS Pro: `--single-node-vni`

The above options will ensure that a job-level VNI is provisioned and
available for Margo to use.  Margo processes will be able to communicate
with each other freely as long as they are launched within the same
scheduler job (they do *not* need to be launche with the same `srun` or
`mpiexec` invocation).

Using the system VNI (if available)
-----------------------------------
If a system-level VNI is available (see above for how to check), then Margo
will use it automatically if you launch processes directly.  You can also
use it in processes launched via `mpiexec` by specifying the `--no-vni` option,
or use it in jobs launched via `srun` by specifying the `--network no_vni`
option.  These options prevent the resource manager from provisioning a VNI
for the job, leaving Margo to use the default system VNI.

Other configurations (rare)
---------------------------
You can also explicitly specify a VNI for Margo to use by setting the
`auth_key` json parameter.  See the :ref:`JSON configuration documentation <_margo_09_config>` for details.
