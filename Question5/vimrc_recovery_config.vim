" ============================================================
" File: .vimrc (Optimal crash-recovery configuration for vi/vim)
" Purpose: Demonstrates the recommended vim settings for Question 5
" ============================================================

" --- Swap File Settings (Primary Recovery Mechanism) ---
set swapfile
set updatetime=2000       " Write swap file every 2 seconds
set updatecount=100       " Write swap every 100 keystrokes
set directory=~/.vim/swap//   " Centralise swap files (// = unique names)

" --- Persistent Undo (Secondary Recovery Mechanism) ---
set undofile
set undodir=~/.vim/undodir
set undolevels=1000        " Maximum number of undo operations
set undoreload=10000       " Lines to save for undo on buffer reload

" --- Backup Files (Tertiary Safety Net) ---
set backup
set backupdir=~/.vim/backup//
set backupext=.bak
set writebackup            " Create backup before overwriting

" --- General Quality of Life ---
set number                 " Show line numbers
set autoread               " Auto-reload if file changed externally
set confirm                " Prompt before losing unsaved changes

" To create required directories, run once in terminal:
" mkdir -p ~/.vim/swap ~/.vim/undodir ~/.vim/backup
