syntax on
filetype plugin on

set nocompatible
set backspace=2
set ruler
set showmatch
set showmode

set tabstop=4
set shiftwidth=4
set smarttab
" set softtabstop=4

" expand tab to spaces
set expandtab

set autoindent
set smartindent
set cindent
set cinoptions=:0,g0,t0,(0,Ws,m1

set hlsearch
set incsearch
set smartcase

" statusline
set statusline=%<%f\ %h%m%r%=%k[%{(&fenc==\"\")?&enc:&fenc}%{(&bomb?\",BOM\":\"\")}]\ %-14.(%l,%c%V%)\ %P

" remove search high light
map <F2> :silent! nohlsearch<CR>

" Jump to previous compile error
map <F3> :cp<CR>

" Jump to next compile error
map <F4> :cn<CR>

" run make command
map <F5> :make<CR>

" run make clean command
map <F6> :make -s clean<CR>

" alt .h .cpp
map <F7> :A<CR>

nnoremap <silent> <F8> :TlistToggle<CR>
map <F10> :NERDTreeToggle<CR>
imap <F10> <ESC> :NERDTreeToggle<CR>

" press F11 to toggle paste mode
set pastetoggle=<F11>

" remove trailing spaces
function! RemoveTrailingSpace()
	if $VIM_HATE_SPACE_ERRORS != '0' &&
				\(&filetype == 'c' || &filetype == 'cpp' || &filetype == 'vim')
		normal m`
		silent! :%s/\s\+$//e
		normal ``
	endif
endfunction

" apply GNU indent in system dir
function! GnuIndent()
	setlocal cinoptions=>4,n-2,{2,^-2,:2,=2,g0,h2,p5,t0,+2,(0,u0,w1,m1
	setlocal shiftwidth=2
	setlocal tabstop=8
endfunction

" fix inconsist file format
function! FixInconsistFileFormat()
	if &fileformat == 'unix'
		silent! :%s/\r$//e
	endif
endfunction

" Don't indent namespace and fix template indent
function! CppNoNamespaceAndTemplateIndent()
	let l:cline_num = line('.')
	let l:cline = getline(l:cline_num)
	let l:pline_num = prevnonblank(l:cline_num - 1)
	let l:pline = getline(l:pline_num)
	while l:pline =~# '\(^\s*{\s*\|^\s*//\|^\s*/\*\|\*/\s*$\)'
		let l:pline_num = prevnonblank(l:pline_num - 1)
		let l:pline = getline(l:pline_num)
	endwhile
	let l:retv = cindent('.')
	let l:pindent = indent(l:pline_num)
	if l:pline =~# '^\s*template\s*<\s*$'
		let l:retv = l:pindent + &shiftwidth
	elseif l:pline =~# '^\s*template\s*<.*>\s*$'
		let l:retv = l:pindent
	elseif l:pline =~# '\s*typename\s*.*,\s*$'
		let l:retv = l:pindent
	elseif l:pline =~# '\s*typename\s*.*>\s*$'
		let l:retv = l:pindent - &shiftwidth
	elseif l:cline =~# '^\s*>\s*$'
		let l:retv = l:pindent - &shiftwidth
	elseif l:pline =~# '^\s*namespace.*'
		let l:retv = 0
	endif
	return l:retv
endfunction

if has("autocmd")
	filetype plugin indent on

	" When editing a file, always jump to the last cursor position
	autocmd BufReadPost *
				\ if line("'\"") > 0 && line ("'\"") <= line("$") |
				\ exe "normal g'\"" |
				\ endif

	autocmd BufEnter /usr/include/c++/* setfiletype cpp
	autocmd BufEnter /usr/include/* call GnuIndent()
	autocmd BufWritePre * call RemoveTrailingSpace()
	autocmd BufWritePre * call FixInconsistFileFormat()
	autocmd BufEnter *.{cc,cxx,cpp,h,hh,hpp,hxx} setlocal indentexpr=CppNoNamespaceAndTemplateIndent()
endif

function MyDiff()
  let opt = '-a --binary '
  if &diffopt =~ 'icase' | let opt = opt . '-i ' | endif
  if &diffopt =~ 'iwhite' | let opt = opt . '-b ' | endif
  let arg1 = v:fname_in
  if arg1 =~ ' ' | let arg1 = '"' . arg1 . '"' | endif
  let arg2 = v:fname_new
  if arg2 =~ ' ' | let arg2 = '"' . arg2 . '"' | endif
  let arg3 = v:fname_out
  if arg3 =~ ' ' | let arg3 = '"' . arg3 . '"' | endif
  let eq = ''
  if $VIMRUNTIME =~ ' '
    if &sh =~ '\<cmd'
      let cmd = '""' . $VIMRUNTIME . '\diff"'
      let eq = '"'
    else
      let cmd = substitute($VIMRUNTIME, ' ', '" ', '') . '\diff"'
    endif
  else
    let cmd = $VIMRUNTIME . '\diff'
  endif
  silent execute '!' . cmd . ' ' . opt . arg1 . ' ' . arg2 . ' > ' . arg3 . eq
endfunction

if has('win32')
    set diffexpr=MyDiff()
endif

" aoto detect file encoding
set fileencodings=ucs-bom,utf-8-bom,utf-8,cp936,gb18030,ucs,big5

let c_gnu = 1

" high light mixed space tab error
let c_space_errors = 1
"let c_no_tab_space_error = 1

" gcc sentence expression ({x, y;}) does not show as error
let c_no_curly_error = 1

" high light characters over 80 columns
match DiffAdd '\%>80v.*'

" allow log column width
if has("autocmd")
	au BufEnter *.log match DiffAdd '\%>1024v.*'
endif

" high light trialing spaces
syn match Error '\s\+$'

augroup filetype
   au! BufRead,BufNewFile *.proto setfiletype proto
   au BufRead,BufNewFile *.thrift set filetype=thrift
augroup end

" load doxygen syntex hilight support in c/c++ source code
let g:load_doxygen_syntax=1
let g:NeoComplCache_EnableAtStartup = 1

" show 'tab' character explicitly
set listchars=tab:>-,trail:-
set list

" set termencoding according to locale automatically
let &termencoding = substitute($LC_ALL, "[a-zA-Z_-]*\.", "", "")

let g:fencview_autodetect = 1

" call cpplint.py to check code style
function CheckStyle()
    let l:old_makeprg = &makeprg
    setlocal makeprg=cpplint.py
    make %
    let &makeprg=old_makeprg
endfunction

" CheckStyle command, usable in command mode
command! CheckStyle call CheckStyle()

