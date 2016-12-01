fusermount -u home
sshfs -p 51800 s49517@remote.vdi.mipt.ru:. home -oauto_cache,reconnect,transform_symlinks,follow_symlinks
