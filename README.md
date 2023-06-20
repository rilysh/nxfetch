## nxfetch
A fetch utility for linux systems

## Usage
To fetch information, normally you'd do `nxfetch --all` or `nxfetch --textall` (for text only)\
Run `nxfetch --help` for more information. Also provided below.
```
Usage
--all - fetch information with OS ascii icon, if available
--textall - fetch information without ascii icon
--user - current logged in username
--os - currently running unix-based OS name
--cpu - currently running CPU name
--kernel - kernel version
--host - currently running host name
--uptime - systemwide uptime
--pkgcnt - count numbers of packages installed
--shell - default logged in users' shell
--dwm - default DE or WM, if available
--gpu - single GPU vendor name
--memory - total RAM and free RAM
--swap - total swap and free swap
```

### Issues
Not all distributions logo has been merged here yet, thus it will be common to see a generic linux (penguin) logo instead of a distribution logo. Until I'm merging all neofetch logos, if you wish you can contribute by creating one or more distributions logos.

Note: You don't exactly have to create the ASCII logo rather take it from neofetch. Only logos with `*_small` suffixes are required.

#### Ascii Art
Distribution ascii arts were picked up from [neofetch](https://github.com/dylanaraps/neofetch).
Dog's paw was picked from https://emojicombos.com/dog