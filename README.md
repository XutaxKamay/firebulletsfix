# firebulletsfix

Fixes shooting position problem displaced by one tick.

## Requirements

- [DHooks2](https://github.com/peace-maker/DHooks2)

## TODO
- Replace GetClientEyePosition by Weapon_ShootPosition, cause GetClientEyePosition might not give proper results.
- Check the return address inside the Weapon_ShootPosition function hook to know if it's really FX_FireBullets calling it. (l4d2 bugs, or others)
