---
tags:
  - command
---

# /autoaccept

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/autoaccept [option] [setting]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Controls, displays status, and configures MQ2AutoAccept
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `(no option)` | Lists command syntax |
| `on|off` | Main accept toggle. Nothing else will accept if this is off. |
| `translocate on|off` | Toggle acceptance of translocate or zephyr port.  Default OFF |
| `anchor on|off` | Toggle acceptance of primary/secondary real estate anchor port.  OFF |
| `selfanchor on|off` | Toggle acceptance of primary/secondary real estate anchor port when you cast it. Default OFF |
| `trade on|off` | Toggle acceptance of trades by people on the auto accept list. Default ON |
| `trade always on|off` | Toggles always accept all trades. Default OFF |
| `trade reject on|off` | Reject trades for people not on auto accept list after 5 seconds. Default OFF |
| `group on|off` | Toggles accept group invites. Default ON |
| `fellowship on|off` | Toggles accept fellowship invites. Default ON |
| `raid on|off` | Toggles accept raid invites. Default ON |
| `status` | Lists status of toggles. |
| `list` | Lists users on your auto accept list. |
| `save` | Saves settings to ini. Changes DO NOT auto save. |
| `load` | load settings from ini |
| `add <NAME>` | Add NAME to the auto accept list. |
| `del <NAME>` | Delete NAME from your auto accept list. |
| `addanchor <VALUE>` | Add VALUE as a valid anchor target. Put the entire address inside quotes as it shows in the portal dialog box such as Willow Circle Bay, 100 Vanward Heights |
| `delanchor <VALUE>` | Delete VALUE from your valid anchor target list. Put the entire address inside quotes as it shows in the portal dialog box such as Willow Circle Bay, 100 Vanward Heights |

## Examples

- Add bob to your autoaccept list and save the .ini

  ```eqcommand
  /autoaccept add bob
  
  /autoaccept save
  ```