# name of your application
APPLICATION = node

# If no BOARD is found in the environment, use this default:
#BOARD ?= samr21-xpro
BOARD ?= native
# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT
EXTERNAL_MODULE_DIRS += $(CURDIR)/../packet_logger
INCLUDES += -I$(CURDIR)/../packet_logger

#FEATURES_REQUIRED += riotboot

# Set RIOTBOOT_BUILD to indicate a riotboot application build

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP -Wno-unknown-warning-option


# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# bootloader modules
#USEMODULE += riotboot_hdr
#USEMODULE += riotboot_slot

# network modules
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_sock_udp

USEMODULE += packet_logger

include $(RIOTBASE)/Makefile.include
