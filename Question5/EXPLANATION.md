# Question 5 - vi Editor Recovery After a System Crash

## Scenario

A developer is editing server.conf using vi and the system crashes before they saved. What recovery options are available?

---

## Recovery mechanisms in vi

### 1. Swap Files

When you open any file in vi/vim, it immediately creates a hidden swap file (like .server.conf.swp). This file gets updated every few seconds and after every ~200 keystrokes. It stores your unsaved edits on disk in real time.

When you reopen the file after a crash:
```bash
vim server.conf
```
vim detects the swap file and shows a prompt:
```
E325: ATTENTION
Found a swap file by the name ".server.conf.swp"
    modified: YES
    process id: 4821 (not running)

[O]pen Read-Only, (E)dit anyway, (R)ecover, (Q)uit, (A)bort:
```
Pressing R recovers the file from the swap. You can also list all available swap files with:
```bash
vim -r
```
Or recover a specific file:
```bash
vim -r server.conf
```

After recovering, save immediately with :w, then delete the stale swap file:
```bash
rm .server.conf.swp
```

This is the most useful mechanism for crash recovery because it captures edits that were never saved.

---

### 2. Undo History (persistent undo)

vim supports persistent undo through the undofile option. If enabled in ~/.vimrc:
```vim
set undofile
set undodir=~/.vim/undodir
```
vim saves the full undo tree to a file like .server.conf.un~ even after you close it. On next open, you can navigate back through previous states using:
```
u            " undo one step
Ctrl+R       " redo
:earlier 5m  " go back to state 5 minutes ago
:undolist     " see the full undo tree
```

This is useful if swap file recovery gives you a state that's still wrong - you can step back further through the edit history.

Limitation: it's not enabled by default, you have to configure it. Also only available in vim/neovim, not the original vi.

---

### 3. Registers

vim uses registers to store copied/deleted text. The unnamed register "" holds the last yanked text, registers "1-"9 hold deletion history. You can view them with:
```
:registers
```

However, registers are stored in memory, not on disk. They are lost when vim exits - whether it crashes or closes normally. So registers are not useful for crash recovery by themselves. They can help within a session (paste something you deleted earlier) but once vim dies, register contents are gone.

The viminfo file (~/.viminfo) saves some register contents between clean sessions but is not written during a crash.

---

### 4. Backup Files

With the right settings in ~/.vimrc:
```vim
set backup
set backupdir=~/.vim/backup//
set backupext=.bak
```
vim keeps a copy of the file as it was before your last save. So if server.conf was last saved at 12:30 and you crash at 12:38, the backup has the 12:30 version.

This is different from swap files - the backup has the last-saved version, while the swap has the latest edits. The backup is useful if the recovered file is somehow corrupt and you want to go back to the last clean save.

To restore from backup:
```bash
cp ~/.vim/backup/server.conf.bak server.conf
```

---

### 5. Auto-recovery / autoread

```vim
set autoread
```
autoread tells vim to automatically reload the file if it changes on disk while vim is open. This is for cases where another process modifies the file externally, not for crash recovery. It doesn't help after a crash because vim has already exited.

It's more of a collaboration feature than a crash recovery tool.

---

## Comparison

| Mechanism | Survives crash | Needs config | What it contains |
|-----------|---------------|--------------|-----------------|
| Swap file | Yes | No (automatic) | Edits since last save |
| Undo file | Yes | Yes | Full edit history |
| Registers | No | No | Last yanked/deleted text (memory only) |
| Backup file | Yes | Yes | Last saved version |
| autoread | No | No | Not applicable |

---

## My proposed recovery strategy

The most reliable approach is to use all three disk-based mechanisms together:

**Immediate step after crash:**
```bash
vim -r server.conf    # use the swap file
```
Press R to recover. The swap file contains edits up to a few seconds before the crash - usually very little is lost.

**If the recovered state is inconsistent:**
```
:earlier 5m    " step back through undo history to a known-good state
```
This requires the undofile feature to be enabled beforehand.

**Last resort:**
```bash
cp ~/.vim/backup/server.conf.bak server.conf    # restore last clean save
```

**Recommended .vimrc settings to set up in advance:**
```vim
set swapfile
set updatetime=2000        " write swap every 2 seconds instead of 4
set undofile
set undodir=~/.vim/undodir
set backup
set backupdir=~/.vim/backup//
```

**Why this strategy is the most reliable:**

The swap file is the primary tool because it's automatic and covers unsaved changes - which is exactly what's lost in a crash. Persistent undo adds a safety net in case the recovered state needs further adjustment. Backup files protect against the situation where the recovery itself goes wrong.

Registers are unreliable for crash recovery because they live only in memory. autoread solves a different problem entirely.

The swap file alone covers most real crash scenarios. Adding the undo file means you can navigate to any point in the edit history. The combination of all three gives the highest chance of recovering exactly the state you were at before the crash.
