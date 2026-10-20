# Editor de Texto - 3DShell

## Descripción
El editor de texto integrado permite abrir y editar archivos de texto plano directamente desde 3DShell. Está diseñado como un editor simple similar a Notepad, optimizado para la interfaz de Nintendo 3DS.

Se puede abrir de dos formas:
- Desde el **app drawer**: la app **Notepad** abre un documento nuevo en blanco.
- Desde **File Explorer**: al seleccionar un archivo de texto.

## Archivos Soportados
Al seleccionar en File Explorer un archivo con alguna de estas extensiones, se abre en el editor:
- `.txt` - Archivos de texto plano
- `.log` - Archivos de registro
- `.json` - Archivos JSON
- `.xml` - Archivos XML
- `.ini` - Archivos de configuración INI
- `.cfg` - Archivos de configuración
- `.md` - Archivos Markdown
- `.cpp`, `.c` - Código fuente C/C++
- `.h`, `.hpp` - Headers C/C++
- `.py` - Scripts Python
- `.js` - Scripts JavaScript
- `.html` - Páginas HTML
- `.css` - Hojas de estilo CSS
- `.csv` - Tablas CSV
- `.yml`, `.yaml` - Archivos YAML
- `.toml` - Archivos de configuración TOML
- `.lua` - Scripts Lua
- `.sh` - Scripts de shell
- `.rs` - Código fuente Rust
- `.s`, `.asm` - Código ensamblador

## Interfaz del Editor

### Modo Vista (View)
- **Pantalla superior**: Muestra el contenido del archivo
- **Pantalla inferior**: Título "Notepad" y botones táctiles Edit, Save y Back. Muestra "* unsaved" si hay cambios sin guardar
- **Barra de título**: Nombre del archivo (con "*" si hay cambios sin guardar)
- **Barra de estado**: Posición del cursor (línea y columna) y modo actual
- **Controles**: A: editar, X: guardar, B: salir del documento (pregunta si hay cambios sin guardar), D-pad: mover el cursor

### Modo Edición (Edit)
- **Pantalla superior**: Editor de texto con cursor visible
- **Pantalla inferior**: Teclado virtual personalizado con múltiples layouts
- **Cursor visible**: Parpadea en la posición actual
- **Navegación**: D-pad para mover el cursor
- **Controles:** A o X: guardar, B: volver al modo vista (conserva los cambios), START: guardar y volver al modo vista, L+A: editar la línea actual con el teclado del sistema (solo en modo edición; SELECT está reservado para el Task Manager)
- **Teclado virtual**: Se usa con el touch en la pantalla inferior; ver la sección de teclado más abajo

## Funcionalidades

### Navegación
- **Flechas direccionales**: Mover cursor por línea/columna
- **Scroll automático**: La vista se desplaza cuando el cursor se acerca al borde

### Edición
- **Teclado virtual personalizado**: 
  - Layout QWERTY con letras minúsculas
  - Layout con letras mayúsculas (presionar SHIFT)
  - Layout numérico (presionar 123)
  - Layout de símbolos (presionar #+=)
  - Fila inferior: SHIFT, 123, espacio, BKSP y ENTER
- **Inserción**: El texto se inserta en la posición del cursor
- **Borrado**: Retroceso elimina caracteres hacia atrás
- **Nueva línea**: Enter crea una nueva línea

### Archivo
- **Carga automática**: Al abrir un archivo, se carga su contenido
- **Guardado manual**: X en modo vista; X, A o START en modo edición (START además vuelve al modo vista). Ver [Guardado y Diálogo de Archivos](#guardado-y-diálogo-de-archivos)
- **Documento nuevo**: Al abrir Notepad desde el app drawer se crea un documento en blanco. Guardarlo abre el diálogo de archivos para elegir carpeta y nombre
- **Indicador de cambios**: "*" en el título cuando hay modificaciones sin guardar

## Guardado y Diálogo de Archivos

### Guardar
- **Documento con nombre** (abierto desde File Explorer): guarda directamente en el mismo archivo
- **Documento nuevo** (en blanco): abre el diálogo de archivos "Save As"
- **Reemplazo**: si el nombre elegido ya existe, pregunta "Replace file?" (Yes / No). No se sobrescribe sin confirmación

### Cambios sin guardar
Al salir con cambios sin guardar (HOME, X, B en modo vista o el botón Back) aparece la pregunta "Unsaved changes":
- **Save**: guarda y sale. En un documento nuevo, abre el diálogo y sale al confirmar el nombre
- **Discard**: sale sin guardar
- **Cancel**: vuelve al editor

### Diálogo de archivos (reutilizable)
Implementado en `source/shell/gui/screens/file_dialog_screen.h`. Cada aplicación declara qué acciones permite (Open, Save, Select folder, en cualquier combinación) y sus filtros de tipo de archivo.
- **Pantalla superior**: carpeta actual y la lista de carpetas y archivos (`GUI::FolderView`). Se navega solo con el D-pad: UP/DOWN seleccionan, A abre una carpeta o elige un archivo, B sube a la carpeta padre (sin carpeta padre, B cancela)
- **Pantalla inferior**: barra táctil, campo **Nombre**, dropdown **Tipo de archivo** y los botones (Save / Open / Select / Cancel). Los mensajes de error aparecen encima del campo de nombre
- **Nombre**: al tocarlo, o con X, se abre el teclado del sistema. Al cambiar de carpeta el nombre se limpia
- **Tipo de archivo**: dropdown con los filtros, abierto con touch o con L/R. Al cambiar de filtro, la lista muestra solo los archivos que coinciden (las carpetas siempre se muestran). Notepad usa All files, txt, log y json. Si el nombre no tiene extensión, se agrega la primera extensión del filtro elegido (`.txt` con All files)
- Notepad usa Save. Open y Select folder están disponibles para otras aplicaciones, pero todavía ninguna las usa
- La lista no acepta toque: en la pantalla inferior están los controles del diálogo

## Teclado Virtual

### Layouts Disponibles
1. **QWERTY Minúsculas**: Letras minúsculas estándar
2. **QWERTY Mayúsculas**: Letras mayúsculas (accesible con ⇧)
3. **Numérico/Símbolos**: Números y símbolos comunes (accesible con 123)
4. **Símbolos Extendidos**: Símbolos adicionales (accesible con #+=)

### Controles del Teclado
- **Táctil**: Tocar las teclas para insertar caracteres
- **BKSP**: Elimina el carácter anterior
- **Espacio**: Inserta un espacio
- **ENTER**: Inserta una nueva línea
- **SHIFT**: Cambia a mayúsculas (y vuelve a minúsculas)
- **123**: Cambia al layout numérico (y vuelve a letras con **ABC**)
- **#+=**: Cambia al layout de símbolos

### Características
- Diseño optimizado para pantalla táctil de 3DS
- Feedback visual al presionar teclas
- Inserción directa en la posición del cursor del editor
- Al entrar al modo edición, el teclado vuelve al layout de minúsculas

## Limitaciones
- **Líneas máximas**: 1000 líneas por archivo
- **Tamaño máximo**: Al abrir, solo se leen los primeros 256 KB del archivo
- **Longitud de línea**: Limitada a 50 caracteres visibles
- **Codificación**: Solo UTF-8 básico
- **Funciones avanzadas**: Sin búsqueda/reemplazo, deshacer/rehacer
- **Reemplazo sin preguntar**: Abrir un archivo desde File Explorer o crear un documento nuevo desde el app drawer reemplaza el documento cargado sin preguntar. El flujo normal no lo permite, porque el editor pregunta por los cambios antes de salir

## Arquitectura del Código

### Archivos
- `source/shell/notepad/notepad.h` / `notepad.cpp` - Editor: carga, guardado, edición y render. `IsTextFile()` define las extensiones
- `source/shell/gui/screens/notepad_screen.h` / `notepad_screen.cpp` - App de pantalla completa del app drawer (barra táctil, HOME, X)
- `source/shell/gui/screens/text_reader_screen.h` / `text_reader_screen.cpp` - Editor abierto desde File Explorer
- `source/shell/gui/screens/notepad_session.h` - Reglas de guardado y salida compartidas por ambas pantallas
- `source/shell/gui/screens/file_dialog_screen.h` / `file_dialog_screen.cpp` - Diálogo reutilizable Open / Save / Select folder
- `source/shell/controls/folder_view.h` / `folder_view.cpp` - Lista de carpetas y archivos del diálogo (gfx0, D-pad)
- `source/shell/controls/screen_prompt.h` - Pregunta modal a pantalla completa con el tema de la app
- `source/ui/controls/message_box.h` / `source/ui/controls/combo_box.h` - Controles genéricos: pregunta modal y dropdown
- `include/apps.h` - Registro de aplicaciones del app drawer (incluye Notepad)
- `source/shell/osk/virtual_keyboard.h` / `virtual_keyboard.cpp` - Teclado virtual de la pantalla inferior
- `source/shell/gui/screens/file_explorer_screen.cpp` - Abre archivos de texto con `Notepad::IsTextFile()`
- `source/ui/screen_manager.cpp` - Atajo global de SELECT (Task Manager)

### Modos
- `NOTEPAD_VIEW` - Modo de solo lectura (navegación)
- `NOTEPAD_EDIT` - Modo de edición

### Estructura de Datos
```cpp
struct NotepadData {
    std::string filename;            // Nombre del archivo
    std::string filepath;            // Ruta completa (vacía si el documento no tiene nombre)
    std::vector<std::string> lines;  // Contenido por líneas
    int cursor_line;                 // Línea actual del cursor
    int cursor_col;                  // Columna actual del cursor
    int scroll_offset;               // Desplazamiento vertical
    int scroll_x;                    // Desplazamiento horizontal
    NotepadMode mode;                // NOTEPAD_VIEW o NOTEPAD_EDIT
    bool modified;                   // Indicador de cambios sin guardar
    bool open;                       // Hay un documento abierto
};
```

## Uso en el Código
Para integrar nuevos tipos de archivos o modificar el comportamiento:

1. **Agregar extensiones**: Modificar `IsTextFile()` en `source/shell/notepad/notepad.cpp`
2. **Cambiar límites**: Ajustar constantes como `NOTEPAD_MAX_LINES` en `notepad.h`
3. **Agregar funciones**: Extender `Control()` para nuevos atajos
4. **Modificar UI**: Cambiar `DisplayTop()` y `DisplayBottom()` para nueva apariencia

## Consideraciones de Rendimiento
- **Memoria limitada**: Optimizado para archivos pequeños/medianos
- **Renderizado eficiente**: Solo líneas visibles se dibujan
- **Entrada controlada**: Teclado virtual evita sobrecarga

## Atajos de Combinación
- `UI::Chord(k_down, k_held, modifier, button)` en `source/ui/key_chord.h` detecta L+botón en el flanco de presión.
- Notepad lo usa para L+A (editar línea). SELECT no se usa en Notepad porque `source/ui/screen_manager.cpp` lo reserva para el Task Manager.

## Futuras Mejoras
- Soporte para más codificaciones
- Funciones de búsqueda y reemplazo
- Deshacer/rehacer básico
- Resaltado de sintaxis para código
- Soporte para archivos más grandes