"""Resolvers for the `bsp` component.

Every feature here is the software's name for a fact the board carries; the raw
configuration vocabulary (``chip.*``) stops at this file.
"""

from strata import feature


@feature("bsp.FAMILY")
def family(config):
    return config.chip.family


@feature("bsp.BOARD")
def board(config):
    return config.board.name


@feature("bsp.MODEL")
def model(config):
    return config.chip.model


@feature("bsp.ARCH_QINGKE")
def arch_qingke(config):
    return config.chip.family in ("ch57x", "ch32x03x")


@feature("bsp.USB")
def usb(config):
    return config.chip.usb
