" Enable hlint and GHC via Cabal
let g:ale_linters = {'c': ['clangtidy'], 'sh': ['shellcheck'], 'python': ['mypy', 'pylint']}
" ... only
let g:ale_linters_explicit = 1
" Don't lint until I save
let g:ale_lint_on_text_changed = 'never'
let g:ale_lint_on_insert_leave = 0
let g:ale_lint_on_enter = 0

" Configure Neoformat to use clang-format for C
let g:neoformat_c_clangformat = {
      \ 'exe': 'clang-format',
      \ 'args': ['--style=Chromium', '-assume-filename=' . expand('%:t')],
      \ 'stdin': 1,
      \}
let g:neoformat_enabled_c = ['clangformat']

" Configure Neoformat to use yapf for Python
let g:neoformat_enabled_c = ['yapf']

" Treat .h files as C
augroup project
  autocmd!
  autocmd BufRead,BufNewFile *.h,*.c set filetype=c
augroup END

" Enable automagic autoformatting
augroup fmt
  autocmd!
  autocmd BufWritePre * undojoin | Neoformat
augroup end


