(defvar rootDirShort "~/code/current/rltest/")
(defvar rootDir (expand-file-name rootDirShort))

(setq compile-command (concat "(cd " rootDir " && make -k -j && cd -)"))

(defun runProgram ()
  (interactive)
  (shell-command (concat "(cd " rootDir " && ./run && cd -)" ))
  )

(global-set-key (kbd "<f5>") 'compile)
(global-set-key (kbd "<f6>") 'runProgram)

(setq ac-clang-flags
      (append ac-clang-flags (split-string (concat "-I" rootDir "src")
                                           "/usr/include/libtcod")))

