Grup CristoreyII

Marc Melia Flexas
Pau Rosado Muñoz
Xavier Vives Marcus

Hi ha un "petit" error ja que a l'arxiu ficheros_basicos.c hi ha un error amb 
funció traducir_bloque_inodo quan es cridada per la funcio mi_read_f

llavors, al arxiu ficheros_basicos2 hi ha una versió no optimitzada amb la qual 
si es pot dur a terme la funcio mi_read_f.

Tambien hay un fallo en el rediccionamiento del leer, ja que los permisos son
rw-r-r en vez de rw-rw-r.

Ademas de que a la hora de ejecutar el 2 script del nivel 5, no se copia bien el 
texto, ya que se pierde informacion del principio

Quitando estos comportanmientos, no hemos encontrado mas errores.