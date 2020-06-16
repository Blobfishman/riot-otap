# name of your application
APPLICATION = gateway

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
#CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:

USEMODULE += gnrc_netdev_default
USEMODULE += gnrc_ipv6_router
USEMODULE += gnrc_sock_udp
USEMODULE += auto_init_gnrc_netif
USEMODULE += vfs
USEMODULE += shell
USEMODULE += gnrc_icmpv6_echo
USEMODULE += shell_commands
USEMODULE += ps

include $(RIOTBASE)/Makefile.include
