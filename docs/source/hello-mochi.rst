.. You might think we could use sectnum here to auto-number, but it actually
   works pretty poorly to try to apply that to a single page in a broader
   readthedocs tree.  We'll just manually number the artifacts.

.. _hello-mochi-label:
.. highlight:: console

Hello Mochi
===========


.. important::
   *Hello Mochi* should be considered beta quality at this time.  Please
   report feedback to the public mailing list or Slack space (links for
   these can be found on the right column of the
   `Mochi web site <https://www.mcs.anl.gov/research/projects/mochi/>`_).

"Hello Mochi" is a step-by-step procedure for getting started with Mochi on
a new platform.  This document is intended for HPC data service developers
and system software researchers, but it should be equally useful to anyone
who wants to try out the Mochi software stack and does not have access to
pre-configured components.

This document is structured as an ordered set of methodical steps that walk
through the process of building and running Mochi software.  Each step
includes a specific “demonstration artifact” to be collected.  These
demonstration artifacts confirm successful execution or provide detailed
diagnostic data on failure.  If you encounter any problems during the Hello
Mochi process, please share the corresponding demonstration artifact on the
Mochi Slack channel or Mochi mailing list available.  Links to these support
resources can be found on the right hand sidebar of the Mochi web site.

Identify target environment
---------------------------

.. We are kind of abusing admonitions her to get an approximate block format
   we want for each artifact.  It might make more sense to use a "container"
   or maybe even a table in the long run?

The goal of this step is to identify your target platform.  It is especially important to determine what network fabric to use. If at all possible, you should use a high-performance RDMA-capable network such as Infiniband, OmniPath, Slingshot, or Aries.  The Mochi software stack is feature-complete atop TCP/IP as well, but performance will be compromised.


.. admonition:: Artifact 1a

   Provide the system name and (if available) link to online description.

   Example::

      Theta
      https://www.alcf.anl.gov/support/user-guides/theta/hardware-overview/machine-overview/index.html


.. admonition:: Artifact 1b

    Identify the type of network that you plan to use.

    Example::

       Cray Aries (gni)

.. admonition:: Artifact 1c

    Identify an existing “recipe” for your platform.  This usually takes
    the form of a `spack.yaml` file that can be used to specify a Spack
    environment. See
    https://github.com/mochi-hpc-experiments/platform-configurations .
    Note that there is a `generic` subdirectory with documentation on
    how to generate an initial environment file for platforms that do
    not have specific recipes.

    Example::

        https://github.com/mochi-hpc-experiments/platform-configurations/tree/main/ANL/Theta

.. admonition:: Artifact 1d

    What Mercury plugin and address format do you need to use for this
    platform?  Consult the README.md for the recipe from 1c if available
    or see https://mercury-hpc.github.io/user/na/#available-plugins .

    Example::

        ofi
        “ofi+gni://”

Set up Spack
------------

Spack is an HPC-friendly, user-deployable, software package manager.
Although any Mochi component can be installed manually, we recommend using
Spack to simplify the build process and help to manage dependencies.


At this point you should follow the setup instructions at
https://spack.readthedocs.io/en/latest/ or identify a facility-provided
Spack configuration that you can use before collecting Artifact 2.

.. admonition:: Artifact 2

    Demonstrate the ability to install a standalone package in Spack
    (this example package is not directly related to Mochi; the purpose
    is to demonstrate that Spack itself is configured correctly).

    Once you have activated spack, run the following command and show the
    output: “spack install libelf”

    Example:

    .. code-block:: text

        carns-x1-7g ~> spack install libelf
        ==> Installing libelf-0.8.13-6lrbuq5xfnwdeeox7m5g6cz246txdesi
        ==> No binary for libelf-0.8.13-6lrbuq5xfnwdeeox7m5g6cz246txdesi found: installing from source
        ==> Fetching https://mirror.spack.io/_source-cache/archive/59/591a9b4ec81c1f2042a97aa60564e0cb79d041c52faa7416acb38bc95bd2c76d.tar.gz
        ==> No patches needed for libelf
        ==> libelf: Executing phase: 'autoreconf'
        ==> libelf: Executing phase: 'configure'
        ==> libelf: Executing phase: 'build'
        ==> libelf: Executing phase: 'install'
        ==> libelf: Successfully installed libelf-0.8.13-6lrbuq5xfnwdeeox7m5g6cz246txdesi
          Fetch: 1.08s.  Build: 3.13s.  Total: 4.21s.
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/libelf-0.8.13-6lrbuq5xfnwdeeox7m5g6cz246txdesi

Install a package from the Mochi Spack repository
-------------------------------------------------

The Mochi project maintains its own independent repository of packages that
can be added to Spack to gain access to the most up-to-date Mochi software
releases.  This step demonstrates adding this repository into Spack and then
installing a simple Mochi package that is not available in the default
upstream Spack repository.


.. admonition:: Artifact 3

    Demonstrate the ability to install a package from the external Mochi
    repository:  Please run the following commands and show their
    output:

    .. code-block:: bash

        # choose a directory where you will check out a copy of the current Mochi
        #    Spack package repository.  We use /tmp/hello-mochi in this example, but
        #    you should choose a more permanent path.
        $ cd /tmp/hello-mochi
        # clone the repository
        $ git clone https://github.com/mochi-hpc/mochi-spack-packages.git
        # add the repository to spack
        $ spack repo add /tmp/hello-mochi/mochi-spack-packages
        # install an example package that is only available in the Mochi repository
        $ spack install mochi-ch-placement
        # remove repository from spack (we will make this persistent as part of an
        #    environment later)
        $ spack repo rm mochi

    Example:

    .. code-block:: text

        carns-x1-7g ~> cd /tmp/hello-mochi
        carns-x1-7g /t/hello-mochi> git clone https://github.com/mochi-hpc/mochi-spack-packages.git
        Cloning into 'mochi-spack-packages'...
        carns-x1-7g ~> cd /tmp/hello-mochi
        carns-x1-7g /t/hello-mochi> git clone https://github.com/mochi-hpc/mochi-spack-packages.git
        Cloning into 'mochi-spack-packages'...
        remote: Enumerating objects: 4432, done.
        remote: Counting objects: 100% (750/750), done.
        remote: Compressing objects: 100% (92/92), done.
        remote: Total 4432 (delta 637), reused 728 (delta 619), pack-reused 3682
        Receiving objects: 100% (4432/4432), 439.71 KiB | 4.27 MiB/s, done.
        Resolving deltas: 100% (2671/2671), done.
        carns-x1-7g /t/hello-mochi [1]> spack repo add /tmp/hello-mochi/mochi-spack-packages
        ==> Added repo with namespace 'mochi'.
        carns-x1-7g /t/hello-mochi> spack install mochi-ch-placement
        [+] /usr (external autoconf-2.71-x7siqslynwavupcbrqxd3lu5ejfmqw33)
        [+] /usr (external automake-1.16.5-uiai5gcqq4cpcvolmkj4nzddgthvmaje)
        [+] /usr (external libtool-2.4.6-zqwp5nep4ud7vq2yl2oui247k6caerok)
        [+] /usr (external m4-1.4.18-sb5p7lz7gmfh3qba7tf72clw7vbpyhj5)
        ==> Installing mochi-ch-placement-0.1-zwfrofwevasjdo7pk5s3nrqqqb7y3u2t
        ==> No binary for mochi-ch-placement-0.1-zwfrofwevasjdo7pk5s3nrqqqb7y3u2t found: installing from source
        ==> No patches needed for mochi-ch-placement
        ==> mochi-ch-placement: Executing phase: 'autoreconf'
        ==> mochi-ch-placement: Executing phase: 'configure'
        ==> mochi-ch-placement: Executing phase: 'build'
        ==> mochi-ch-placement: Executing phase: 'install'
        ==> mochi-ch-placement: Successfully installed mochi-ch-placement-0.1-zwfrofwevasjdo7pk5s3nrqqqb7y3u2t
          Fetch: 1.28s.  Build: 7.27s.  Total: 8.55s.
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/mochi-ch-placement-0.1-zwfrofwevasjdo7pk5s3nrqqqb7y3u2t
        carns-x1-7g /t/hello-mochi> spack repo rm moch

Create a Spack environment and install the Mochi software stack
---------------------------------------------------------------

A Spack environment is a grouping of software packages and their parameters
(Spack specs) that can be built and deployed as a single coherent unit.  We
recommend using Spack environments to configure a Mochi software spack and
its dependencies.

Spack environments can be constructed ad hoc or they can be specified a
priori using a spack.yaml file.  If you identified a Mochi recipe for your
platform in step 1c, then you may use it now and just modify it so that it
has the correct path to your mochi-spack-packages repo from step 3.  The
example below uses the minimal spack.yaml configuration for a generic system
as found in
https://github.com/mochi-hpc-experiments/platform-configurations/tree/main/generic
.

.. admonition:: Artifact 4

    Demonstrate the ability to create a Spack environment for a Mochi
    software stack and install all packages within it.

    Show the output from the following commands:

    .. code-block:: bash

        # contents of your spack.yaml file
        $ cat spack.yaml
        # create Spack environment (it does not have to be named
        # “hello-mochi”)
        $ spack env create hello-mochi ./spack.yaml
        # activate spack environment
        $ spack env activate hello-mochi
        # install all required software in the environment
        $ spack install

    Example:

    .. code-block:: text

        carns-x1-7g /t/hello-mochi> cat spack.yaml
        # This is a Spack Environment file.
        #
        # It describes a set of packages to be installed, along with
        # configuration settings.
        spack:
          specs:
          - mochi-margo
          repos:
          - /tmp/hello-mochi/mochi-spack-packages
          modules:
                prefix_inspections:
                  lib: [LD_LIBRARY_PATH]
                  lib64: [LD_LIBRARY_PATH]
          packages:
                libfabric:
                  variants: fabrics=rxm,tcp
                mercury:
                  variants: ~boostsys ~checksum
          view: true
        carns-x1-7g /t/hello-mochi> spack env create hello-mochi ./spack.yaml
        ==> Created environment 'hello-mochi' in /home/carns/working/src/spack/var/spack/environments/hello-mochi
        ==> You can activate this environment with:
        ==>   spack env activate hello-mochi
        carns-x1-7g /t/hello-mochi> spack env activate hello-mochi
        carns-x1-7g /t/hello-mochi> spack install
        ==> Starting concretization
        ==> Environment concretized in 21.16 seconds.
        ==> Concretized mochi-margo
         -   mjvnk7r  mochi-margo@0.9.10%gcc@11.2.0~pvar arch=linux-ubuntu22.04-skylake
         -   hxtr2qr          ^argobots@1.1%gcc@11.2.0~affinity~debug~lazy_stack_alloc+perf~stackunwind~tool~valgrind stackguard=none arch=linux-ubuntu22.04-skylake
        [+]  x7siqsl          ^autoconf@2.71%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  uiai5gc          ^automake@1.16.5%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  zcjnfnh          ^json-c@0.16%gcc@11.2.0~ipo build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  qjgfk6r              ^cmake@3.22.1%gcc@11.2.0~doc+ncurses+ownlibs~qt build_type=Release arch=linux-ubuntu22.04-skylake
        [+]  zqwp5ne          ^libtool@2.4.6%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  sb5p7lz          ^m4@1.4.18%gcc@11.2.0+sigsegv patches=3877ab5,fc9b616 arch=linux-ubuntu22.04-skylake
        [+]  6auh76q          ^mercury@2.2.0%gcc@11.2.0 cflags="-fsanitize=address -fno-omit-frame-pointer -g -Wall" ldflags="-fsanitize=address" ~bmi~boostsys~checksum~debug~hwloc~ipo~mpi+ofi~psm~psm2+shared+sm~ucx~udreg build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  di35yy4              ^cmake@3.22.1%gcc@11.2.0 cflags="-fsanitize=address -fno-omit-frame-pointer -g -Wall" ldflags="-fsanitize=address" ~doc+ncurses+ownlibs~qt build_type=Release arch=linux-ubuntu22.04-skylake
        [+]  d6moqm3              ^libfabric@1.15.1%gcc@11.2.0 cflags="-fsanitize=address -fno-omit-frame-pointer -g -Wall" ldflags="-fsanitize=address" ~debug~disable-spinlocks~kdreg fabrics=rxm,sockets,tcp arch=linux-ubuntu22.04-skylake
        [+]  uvo7kjt          ^pkgconf@1.8.0%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        ==> Installing environment hello-mochi
        ==> Installing argobots-1.1-hxtr2qrrl7jxdhtw5ccdryqcajee3opu
        ==> No binary for argobots-1.1-hxtr2qrrl7jxdhtw5ccdryqcajee3opu found: installing from source
        ==> Using cached archive: /home/carns/working/src/spack/var/spack/cache/_source-cache/archive/f0/f0f971196fc8354881681c2282a2f2adb6d48ff5e84cf820ca657daad1549005.tar.gz
        ==> No patches needed for argobots
        ==> argobots: Executing phase: 'autoreconf'
        ==> argobots: Executing phase: 'configure'
        ==> argobots: Executing phase: 'build'
        ==> argobots: Executing phase: 'install'
        ==> argobots: Successfully installed argobots-1.1-hxtr2qrrl7jxdhtw5ccdryqcajee3opu
          Fetch: 0.01s.  Build: 7.35s.  Total: 7.35s.
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/argobots-1.1-hxtr2qrrl7jxdhtw5ccdryqcajee3opu
        [+] /usr (external autoconf-2.71-x7siqslynwavupcbrqxd3lu5ejfmqw33)
        [+] /usr (external automake-1.16.5-uiai5gcqq4cpcvolmkj4nzddgthvmaje)
        [+] /usr (external cmake-3.22.1-qjgfk6rh2ufs3glsf23k4civixmor4lb)
        [+] /usr (external libtool-2.4.6-zqwp5nep4ud7vq2yl2oui247k6caerok)
        [+] /usr (external m4-1.4.18-sb5p7lz7gmfh3qba7tf72clw7vbpyhj5)
        [+] /usr (external cmake-3.22.1-di35yy4kw5bgh3hwsvg7ktmx3upxjx7p)
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/libfabric-1.15.1-d6moqm3s563gl33dvu4mdvwxx6gzxhgb
        [+] /usr (external pkgconf-1.8.0-uvo7kjt22xqtelv2hnnsttxvdmtlbkxu)
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/json-c-0.16-zcjnfnhsomwk5pe43f5dyb3my745n4eh
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/mercury-2.2.0-6auh76quhurqu4kl6beyqb6ls77fe6eh
        ==> Installing mochi-margo-0.9.10-mjvnk7rdry744qhot4z4c6fkmwg7l5le
        ==> No binary for mochi-margo-0.9.10-mjvnk7rdry744qhot4z4c6fkmwg7l5le found: installing from source
        ==> Fetching https://mirror.spack.io/_source-cache/archive/b2/b205b45fe200d1b2801ea3b913fa75d709af97abf470f4ad72a08d2839f03772.tar.gz
        ==> No patches needed for mochi-margo
        ==> mochi-margo: Executing phase: 'autoreconf'
        ==> mochi-margo: Executing phase: 'configure'
        ==> mochi-margo: Executing phase: 'build'
        ==> mochi-margo: Executing phase: 'install'
        ==> mochi-margo: Successfully installed mochi-margo-0.9.10-mjvnk7rdry744qhot4z4c6fkmwg7l5le
          Fetch: 0.58s.  Build: 11.50s.  Total: 12.08s.
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/mochi-margo-0.9.10-mjvnk7rdry744qhot4z4c6fkmwg7l5le
        ==> Updating view at /home/carns/working/src/spack/var/spack/environments/hello-mochi/.spack-env/view

Confirm runtime network initialization
--------------------------------------

You should now have a complete Mochi runtime environment available for use.
The next step is to confirm that this environment is able to initialize the
desired network transport.  It is crucial to use the preferred (ideally,
RDMA-capable) network protocol for your environment in order to extract
maximum performance from Mochi.


Important note: at this point in the Hello Mochi procedure we will begin
executing runtime commands that exercise Mochi components.  If you are using
a platform in which the compute nodes and login nodes are not homogenous,
then you will need to run the prescribed commands on a compute node (for
example, within an interactive job allocation). The login node may not
possess the same network transport or even the same CPU architecture as the
compute nodes on some systems.

.. admonition:: Artifact 5

    Show the result of executing the `margo-info` diagnostic utility.
    Confirm that your desired network transport is activated
    successfully (green).

    Example:

    .. code-block:: text

        ####################################################################
        # Available Margo (Mercury) network transports on host carns-x1-7g
        # - GREEN indicates that it can be initialized successfully.
        # - RED indicates that it cannot.
        ####################################################################

        # <address> <transport> <protocol> <results> <example runtime address>

        ### libfabric tcp provider (TCP/IP) ###
        ofi+tcp://    ofi    tcp    YES    ofi+tcp;ofi_rxm://192.168.122.1:37557
        ### libfabric sockets provider (TCP/IP) ###
        ofi+sockets://    ofi    shm    YES    ofi+sockets://192.168.122.1:34043
        ### integrated sm plugin (shared memory) ###
        na+sm://    na    sm    YES    na+sm://599915-0
        ### TCP/IP protocol, transport not specified ###
        tcp://    <any>    tcp    YES    ofi+tcp;ofi_rxm://192.168.122.1:39137
        ### shared memory protocol, transport not specified ###
        sm://    <any>    sm    YES    na+sm://599915-1
        ### libfabric Verbs provider (InfiniBand or RoCE) ###
        ofi+verbs://    ofi    verbs    NO    N/A
        ### libfabric shm provider (shared memory) ###
        ofi+shm://    ofi    shm    NO    N/A
        ### libfabric PSM2 provider (OmniPath ###
        ofi+psm2://    ofi    psm2    NO    N/A
        ### libfabric OPX provide (OmniPath) ###
        ofi+opx://    ofi    opx    NO    N/A
        ### libfabric GNI provider (Cray Aries) ###
        ofi+gni://    ofi    gni    NO    N/A
        ### libfabric CXI provider (HPE Cassini/Slingshot 11) ###
        ofi+cxi://    ofi    cxi    NO    N/A
        ### integrated PSM plugin (OmniPath) ###
        psm+psm://    psm    psm    NO    N/A
        ### integrated PSM2 plugin (OmniPath) ###
        psm2+psm2://    psm2    psm2    NO    N/A
        ### BMI tcp module (TCP/IP) ###
        bmi+tcp://    bmi    tcp    NO    N/A
        ### UCX TCP/IP ###
        ucx+tcp://    ucx    tcp    NO    N/A
        ### UCX Verbs ###
        ucx+verbs://    ucx    verbs    NO    N/A
        ### UCX automatic transport ###
        ucx+all://    ucx    <any>    NO    N/A
        ### Verbs protocol, transport not specified ###
        verbs://    <any>    verbs    NO    N/A
        ### PSM2 protocol, transport not specified ###
        psm2://    <any>    psm2    NO    N/A

        ####################################################################
        # Notes on interpretting margo-info output:
        # - This utility queries software stack capability, not hardware availability.
        # - For more information about a particular address specifier, please
        #   execute margo-info with that address specifier as its only argument
        #   and check the resulting log file for details.
        #   (E.g., "margo-info ofi+verbs://" for Verbs-specific diagnostics)
        #
        ####################################################################
        # Suggested transport-level diagnostic tools:
        # - libfabric:    `fi_info -t FI_EP_RDM`
        # - UCX:    `ucx_info -d`
        # - verbs:    `ibstat`
        # - TCP/IP:    `ifconfig`
        # - CXI:    `cxi_stat`
        #
        ####################################################################
        # Verbose margo-info information:
        # - debug log output:
        #   /tmp/margo-info-stderr-aTujCI
        # - results in JSON format:
        #   /tmp/margo-info-json-rKsotr
        #
        ####################################################################

Start a server process
----------------------

At this point you have identified your network transport and confirmed that
it is available at runtime.  You should now be able to start a server
process that listens for incoming RPCs on that network fabric. In order to
confirm this capability, we now install the mochi-bedrock bootstrapping
system.  Among other things, bedrock provides a skeleton daemon that can be
used to dynamically load additional service providers.

.. admonition:: Artifact 6

    Demonstrate the ability to install and launch a bedrock server daemon.
    Execute the following commands and show the output:

    .. code-block:: bash

        # add mochi-bedrock as a root spec in your environment
        $ spack add mochi-bedrock
        # install new packages
        $ spack install
        # launch the server, substituting the command line argument with the
        #   appropriate address string from Artifact 1d and 5
        $ bedrock tcp://

    Example:

    .. code-block:: text

        carns-x1-7g ~> spack add mochi-bedrock
        ==> Adding mochi-bedrock to environment hello-mochi
        carns-x1-7g ~> spack install
        ==> Starting concretization
        ==> Environment concretized in 18.32 seconds.
        ==> Concretized mochi-margo
        [+]  yvp5cp6  mochi-margo@0.9.10%gcc@11.2.0~pvar arch=linux-ubuntu22.04-skylake
        [+]  hxtr2qr          ^argobots@1.1%gcc@11.2.0~affinity~debug~lazy_stack_alloc+perf~stackunwind~tool~valgrind stackguard=none arch=linux-ubuntu22.04-skylake
        [+]  x7siqsl          ^autoconf@2.71%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  uiai5gc          ^automake@1.16.5%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  2pqprp2          ^json-c@0.16%gcc@11.2.0~ipo build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  xuzv4eo              ^cmake@3.22.1%gcc@11.2.0~doc+ncurses+ownlibs~qt build_type=Release arch=linux-ubuntu22.04-skylake
        [+]  zqwp5ne          ^libtool@2.4.6%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  sb5p7lz          ^m4@1.4.18%gcc@11.2.0+sigsegv patches=3877ab5,fc9b616 arch=linux-ubuntu22.04-skylake
        [+]  7xpep5r          ^mercury@2.2.0%gcc@11.2.0~bmi~boostsys~checksum~debug~hwloc~ipo~mpi+ofi~psm~psm2+shared+sm~ucx~udreg build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  2y7oqvk              ^libfabric@1.15.1%gcc@11.2.0~debug~disable-spinlocks~kdreg fabrics=rxm,tcp arch=linux-ubuntu22.04-skylake
        [+]  x7yeuj2          ^pkg-config@0.29.2%gcc@11.2.0+internal_glib arch=linux-ubuntu22.04-skylake
        ==> Concretized mochi-bedrock
         -   gyni5wv  mochi-bedrock@0.5.2%gcc@11.2.0+abtio~ipo~mona~mpi build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  xuzv4eo          ^cmake@3.22.1%gcc@11.2.0~doc+ncurses+ownlibs~qt build_type=Release arch=linux-ubuntu22.04-skylake
         -   kqupezf          ^fmt@8.1.1%gcc@11.2.0~ipo+pic~shared build_type=RelWithDebInfo cxxstd=11 arch=linux-ubuntu22.04-skylake
         -   euf7mld          ^mochi-abt-io@0.5.1%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  hxtr2qr              ^argobots@1.1%gcc@11.2.0~affinity~debug~lazy_stack_alloc+perf~stackunwind~tool~valgrind stackguard=none arch=linux-ubuntu22.04-skylake
        [+]  x7siqsl              ^autoconf@2.71%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  uiai5gc              ^automake@1.16.5%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  2pqprp2              ^json-c@0.16%gcc@11.2.0~ipo build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  zqwp5ne              ^libtool@2.4.6%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        [+]  sb5p7lz              ^m4@1.4.18%gcc@11.2.0+sigsegv patches=3877ab5,fc9b616 arch=linux-ubuntu22.04-skylake
         -   7ejpj2i              ^openssl@1.1.1q%gcc@11.2.0~docs~shared certs=mozilla patches=3fdcf2d arch=linux-ubuntu22.04-skylake
         -   3koqnyy                  ^ca-certificates-mozilla@2022-07-19%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
         -   iijqwy5                  ^perl@5.32.1%gcc@11.2.0~cpanm+shared+threads arch=linux-ubuntu22.04-skylake
         -   y3vm6i6                  ^zlib@1.2.12%gcc@11.2.0+optimize+pic+shared patches=0d38234 arch=linux-ubuntu22.04-skylake
        [+]  x7yeuj2              ^pkg-config@0.29.2%gcc@11.2.0+internal_glib arch=linux-ubuntu22.04-skylake
        [+]  yvp5cp6          ^mochi-margo@0.9.10%gcc@11.2.0~pvar arch=linux-ubuntu22.04-skylake
        [+]  7xpep5r              ^mercury@2.2.0%gcc@11.2.0~bmi~boostsys~checksum~debug~hwloc~ipo~mpi+ofi~psm~psm2+shared+sm~ucx~udreg build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
        [+]  2y7oqvk                  ^libfabric@1.15.1%gcc@11.2.0~debug~disable-spinlocks~kdreg fabrics=rxm,tcp arch=linux-ubuntu22.04-skylake
         -   wcburov          ^mochi-thallium@0.10.1%gcc@11.2.0+cereal~ipo build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
         -   e6u5uqj              ^cereal@1.3.2%gcc@11.2.0~ipo build_type=RelWithDebInfo patches=2dfa0bf arch=linux-ubuntu22.04-skylake
         -   wzxn4cn          ^nlohmann-json@3.11.2%gcc@11.2.0~ipo+multiple_headers build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
         -   m6zlvyf          ^spdlog@1.9.2%gcc@11.2.0~ipo+shared build_type=RelWithDebInfo arch=linux-ubuntu22.04-skylake
         -   7gakfr5          ^tclap@1.2.2%gcc@11.2.0 arch=linux-ubuntu22.04-skylake
        ==> Installing environment hello-mochi
        [+] /usr (external cmake-3.22.1-xuzv4eofjfrd7lirlhwuoag5vasidhgn)
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/argobots-1.1-hxtr2qrrl7jxdhtw5ccdryqcajee3opu
        [+] /usr (external autoconf-2.71-x7siqslynwavupcbrqxd3lu5ejfmqw33)
        [+] /usr (external automake-1.16.5-uiai5gcqq4cpcvolmkj4nzddgthvmaje)
        [+] /usr (external libtool-2.4.6-zqwp5nep4ud7vq2yl2oui247k6caerok)
        [+] /usr (external m4-1.4.18-sb5p7lz7gmfh3qba7tf72clw7vbpyhj5)

        # output omitted for clarity

        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/mochi-thallium-0.10.1-wcburovh777u7cyksdybslmtmxurdkch
        ==> Installing mochi-bedrock-0.5.2-gyni5wvfhszkdpj3ja5sgqmiax2amczi
        ==> No binary for mochi-bedrock-0.5.2-gyni5wvfhszkdpj3ja5sgqmiax2amczi found: installing from source
        ==> Using cached archive: /home/carns/working/src/spack/var/spack/cache/_source-cache/archive/4c/4c6d188c43141805c9c9cde6f8f20031437c394a5136f9a97d9561342ac19994.tar.gz
        ==> No patches needed for mochi-bedrock
        ==> mochi-bedrock: Executing phase: 'cmake'
        ==> mochi-bedrock: Executing phase: 'build'
        ==> mochi-bedrock: Executing phase: 'install'
        ==> mochi-bedrock: Successfully installed mochi-bedrock-0.5.2-gyni5wvfhszkdpj3ja5sgqmiax2amczi
          Fetch: 0.00s.  Build: 1m 46.89s.  Total: 1m 46.89s.
        [+] /home/carns/working/src/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.2.0/mochi-bedrock-0.5.2-gyni5wvfhszkdpj3ja5sgqmiax2amczi
        ==> Updating view at /home/carns/working/src/spack/var/spack/environments/hello-mochi/.spack-env/view
        ==> Warning: Skipping external package: mpich@4.0.2%gcc@11.2.0~argobots~benvolio~cuda+fortran+hwloc+hydra+libxml2+pci~rocm+romio~slurm~two_level_namespace~verbs+wrapperrpath datatype-engine=auto device=ch3 netmod=ofi patches=d4c0e99 pmi=pmi arch=linux-ubuntu22.04-skylake/533ib4c
        carns-x1-7g ~> bedrock tcp://
        [2022-09-09 11:42:48.126] [info] Bedrock daemon now running at ofi+tcp;ofi_rxm://192.168.122.1:36773

Access the server from a separate client process
------------------------------------------------

If you leave the daemon from Step 6 running, you can now connect to it.  The
mochi-bedrock package includes a utility called `bedrock-query` that can be
used to retrieve the Mochi configuration from the bedrock server in JSON
format.

.. admonition:: Artifact 7

    Use the runtime address emitted by bedrock in the final line of
    Artifact 7 to connect to the server using the bedrock-query command
    line tool.  This command can be executed from the same node or a
    different node as long as they have network connectivity.  Show the
    output of the following commands (this example activates the
    existing Spack environment in a separate terminal for the client
    executable to use):

    .. code-block:: bash

        # activate Spack environment in another terminal
        $ spack env activate hello-mochi
        # connect to the bedrock process and display its Mochi configuration.  Note
        #    that if the runtime address of the server includes a `;` character you may
        #    need to either escape it with a backslash or put double quotes around the
        #    entire address string.
        $ bedrock-query <transport type> -a <daemon address> -p

    Example:

    .. code-block:: text

        carns-x1-7g ~> spack env activate hello-mochi
        carns-x1-7g ~> bedrock-query tcp:// -p -a ofi+tcp\;ofi_rxm://192.168.122.1:46053
        {
            "ofi+tcp;ofi_rxm://192.168.122.1:46053": {
                "bedrock": {
                    "pool": "__primary__",
                    "provider_id": 0
                },
                "libraries": [],
                "margo": {
                    "argobots": {
                        "abt_mem_max_num_stacks": 8,
                        "abt_thread_stacksize": 2097152,
                        "pools": [
                            {
                                "access": "mpmc",
                                "kind": "fifo_wait",
                                "name": "__primary__"
                            }
                        ],
                        "version": "1.1",
                        "xstreams": [
                            {
                                "affinity": [],
                                "cpubind": -1,
                                "name": "__primary__",
                                "scheduler": {
                                    "pools": [
                                        0
                                    ],
                                    "type": "basic_wait"
                                }
                            }
                        ]
                    },
                    "enable_diagnostics": false,
                    "enable_profiling": false,
                    "handle_cache_size": 32,
                    "mercury": {
                        "address": "ofi+tcp;ofi_rxm://192.168.122.1:46053",
                        "auto_sm": false,
                        "input_eager_size": 4064,
                        "listening": true,
                        "max_contexts": 1,
                        "na_no_block": false,
                        "na_no_retry": false,
                        "no_bulk_eager": false,
                        "no_loopback": false,
                        "output_eager_size": 4080,
                        "request_post_incr": 256,
                        "request_post_init": 256,
                        "stats": false,
                        "version": "2.2.0"
                    },
                    "output_dir": "/home/carns",
                    "profile_sparkline_timeslice_msec": 1000,
                    "progress_pool": 0,
                    "progress_timeout_ub_msec": 100,
                    "rpc_pool": 0,
                    "version": "0.9.10"
                },
                "providers": []
            }
        }

Congratulations!  At this point you have validated the basic functionality of your Margo environment.  Please see the Mochi Read the Docs page for additional information about using more advanced Mochi components.

