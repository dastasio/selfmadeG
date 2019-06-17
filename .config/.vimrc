" TODO: syntax highlighting for NOTEs, TODOs and IMPORTANTs
" TODO: reorder .vimrc
" TODO: maybe implement file backup
syntax on
colorscheme base16-gruvbox-dark-pale
set number
set wrap!
set textwidth=70
set wildmode=list:full
set wildmenu

" Highlighting 70th column
if (exists('+colorcolumn'))
    set colorcolumn=70
    highlight ColorColumn ctermbg=9
endif

" Setting fonts for gvim (others will be added)
if has("gui_running")
  if has("gui_gtk2")
    set guifont=Inconsolata\ 12
  elseif has("gui_macvim")
    set guifont=Menlo\ Regular:h14
  elseif has("gui_win32")
    set guifont=Consolas:h11:cANSI
    set guioptions-=m
    set guioptions-=T
    set guioptions-=r
    set guioptions-=L
  endif
endif

function! SplitOnce()
    if exists("w:IsSplit") == 0
        vsplit
        let w:IsSplit = 1
    endif
endfun

" Split window on open (splits twice in gvim)
au GUIEnter * simalt ~x
au GUIEnter * call SplitOnce()
au VimResized * winc =

nnoremap <silent> <A-k> :wincmd k<CR>
nnoremap <silent> <A-j> :wincmd j<CR>
nnoremap <silent> <A-h> :wincmd h<CR>
nnoremap <silent> <A-l> :wincmd l<CR>

" TODO: syntax highlighting for compile log
" TODO: add searching for build script
" TODO: make this asyncronous
function! Build()
    call setqflist([])
    wall
    echo "Compiling..."
    let Log = system('build')
    cgetexpr Log
    wincmd h
    " TODO: set wrapping for quickfix buffer only
    bel cw
    echon "\r\rCompilation Finished!"
endfunction

" Mapping Alt-m to run build.bat and Alt-n to go to next error
nnoremap <silent> <A-N> :cp<CR>
nnoremap <silent> <A-n> :cn<CR>
nnoremap <silent> <A-m> :call Build()<CR>
nnoremap <silent> <A-c> :call SourceToHeader(0)<CR>
nnoremap <silent> <A-C> :call SourceToHeader(1)<CR>

function! s:SwitchWindow()
    let OldWindow = winnr()
    wincmd l
    if winnr() == OldWindow
        wincmd h
    endif
endfun

function! OpenBufferOrFile(Filename, OtherWindow)
    let s:Bufname = bufname(a:Filename)
    if a:OtherWindow
        call s:SwitchWindow()
    endif
    if (strlen(s:Bufname)) > 0
        w
        exe ":buffer " . s:Bufname
    else
        exe ":e " . a:Filename
    endif
endfun

function! SourceToHeader(OtherWindow)
    if match(expand("%"), '\.cpp') > 0
        let l:Flipname = substitute(expand("%"),'\.cpp\(.*\)','.h\1',"")
    elseif match(expand("%"), '\.h') > 0
        let l:Flipname = substitute(expand("%"),'\.h\(.*\)','.cpp\1',"")
    endif
    call OpenBufferOrFile(l:Flipname, a:OtherWindow)
endfun

" Autoindent options
set expandtab
set shiftwidth=4
set softtabstop=4
set smarttab
set autoindent
set cindent
set cinoptions=l1,g0,N-s,E-s,t0,(0,w1,Ws,m1
" TODO: Maybe add color to statusline
set statusline=\ %f%m\%=\ %y\ %{&fileencoding?&fileencoding:&encoding}\[%{&fileformat}\]\ %p%%\ %l:%c\ 

filetype plugin on
set omnifunc=syntaxcomplete#Complete

function! HeaderSkeleton(Filename)
    let l:HeaderMacro = toupper( substitute( substitute(a:Filename, '\.h', '_H',""), '\.', '_', ""))
    call setline(1, '#if !defined(' . l:HeaderMacro . ')')
    call setline(2, '/* ========================================================================')
    call setline(3, '   $File: $')
    call setline(4, '   $Date: $')
    call setline(5, '   $Revision: $')
    call setline(6, '   $Creator: Casey Muratori $')
    call setline(7, '   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $')
    call setline(8, '   ======================================================================== */')
    call setline(9, '')
    call setline(10, '#define ' . l:HeaderMacro)
    call setline(11, '#endif')
endfun

function! SourceSkeleton()
    call setline(1, '/* ========================================================================')
    call setline(2, '   $File: $')
    call setline(3, '   $Date: $')
    call setline(4, '   $Revision: $')
    call setline(5, '   $Creator: Casey Muratori $')
    call setline(6, '   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $')
    call setline(7, '   ======================================================================== */')
endfun

au BufNewFile *.h   call HeaderSkeleton(expand('%:t'))
au BufNewFile *.c   call SourceSkeleton()
au BufNewFile *.cpp call SourceSkeleton()

" Typing utilities
function! InsertFor(Signed, IndexName, IndexEnd, ...)
    " TODO: Maybe add '[u]int' vs '[u]int32' check
    if a:Signed
        let l:Type = 'int32'
    else
        let l:Type = 'uint32'
    endif
    if a:0 > 0
        let l:IndexStart = a:1
    else
        let l:IndexStart = 0
    endif
    call append(line('.') - 1, [
                \ 'for('. l:Type . ' ' . a:IndexName . ' = ' . l:IndexStart . ';',
                \ '    ' . a:IndexName . ' < ' . a:IndexEnd . ';',
                \ '    ++' . a:IndexName . ')',
                \ '{'])
    normal k
    normal =3k
    normal j
endfun 
command! -nargs=+ Foru call InsertFor(0, <f-args>)
command! -nargs=+ For  call InsertFor(1, <f-args>)

nnoremap ,f :For 
nnoremap ,uf :Foru 
