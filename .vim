function! PostSave()
    :silent !touch .watchfile
    :silent !ctags -R .
endfunction

nmap <leader>` :wall \| call PostSave()<CR>

nmap <leader>1 :silent !touch .runfile<cr>
