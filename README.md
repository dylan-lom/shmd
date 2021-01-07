# **SH**ell **M**ark**D**own

SHMD (Shell Markdown) is an *extension* to the popular [Markdown][1] markup
language

The aim of this project is to allow for execution of shell-script logic in
markdown files at compile time.

For compatability, and reduction of duplicated work, SHMD will be implemented as
a pre-processor for markdown files, so that existing markdown converters
continue to work. Converting a SHMD file to HTML may look something like this

    $ shmd < myshellmarkdownfile.shmd | markdown > myhtmlfile.html

## Features

1. Execution of shell scripts can be achieved with the `$(...)` delimiter, in
   reference to a [command substitution][2].
2. The addition of a header section, where elements in the HTML `<head>` tag can
   be populated, and SHMD functions can be declared. The contents of the header
   will be converted to HTML at compilation.
3. SHMD functions may be declared to simplify inlined scripts, for example to
   allow for succinct execution in other scripting languages.

[1]: https://daringfireball.net/projects/markdown/
[2]: https://www.gnu.org/software/bash/manual/html_node/Command-Substitution.html
