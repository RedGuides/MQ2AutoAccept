---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2autoaccept.97/"
support_link: "https://www.redguides.com/community/threads/mq2autoaccept.66803/"
repository: "https://github.com/RedGuides/MQ2AutoAccept"
config: "MQ2AutoAccept.ini"
authors: "Sym, Knightly, alynel, eqmule, Sic"
tagline: "Accept invites, Trades, Taskadds, Expeditions (dzadds}, trasolocate and primary/second anchors."
---

# MQ2AutoAccept
<!--desc-start-->
This plugin will allow you to create a list of trusted character names that will allow your character to automatically accept Party invites, Raid invites, Trades, Taskadds, Expeditions (dzadds}. Plus auto accept trasolocate and primary/secondary real estate anchor ports.
<!--desc-end-->

## Commands

<a href="cmd-autoaccept/">
{% 
  include-markdown "projects/mq2autoaccept/cmd-autoaccept.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2autoaccept/cmd-autoaccept.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2autoaccept/cmd-autoaccept.md') }}

## Settings

Example MQ2AutoAccept.ini file,

```ini
[Mymage_Settings]
Enabled=1
Translocate=0
Anchor=0
Trade=0
TradeAlways=0
Group=1
Raid=1
[OMymage_Names]
Bob=1
Mike=1
Sally=1
[Mymage_Anchors]
Anchor0=Willow Circle Bay, 100 Vanward Heights
```

Definitions for each setting can be found on the /autoaccept command page.

## Examples

In order to tell all the logged toons to auto accept each other, using MQ2EQBC or MQ2DANNET:

```eqcommand
/noparse /bcaa //bcaa //autoaccept add ${Me.Name}
```

```eqcommand
/bcaa //autoaccept save
```

With mq2dannet, it would be :

```eqcommand
/noparse /dgae /dgae /autoaccept add ${Me.Name}
```

```eqcommand
/dgae /autoaccept save
```
