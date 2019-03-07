;; -*- coding: utf-8 -*-

(let* ((project-root (rh-project-get-root))
       (project-path (rh-project-get-path))
       file-rpath)
  (when project-root
    (setq file-rpath (file-relative-name buffer-file-name project-root))
    (cond
     ((string-match-p "\\.css\\'" file-rpath)
      (rh-setup-css-skewer))
     ((eq major-mode 'c++-mode)
      (set (make-local-variable 'compile-command)
           (concat project-path "build"))))))
