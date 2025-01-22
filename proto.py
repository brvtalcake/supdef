#!/usr/bin/env python3

import lark

SUPDEF_PROTOTYPE_GRAMMAR = """

    start: ( STRING_LIT | CHAR_LIT | c_code | supdef_block | import_statement | comment )*

    INLINE_COMMENT: /\\/\\/[^\\n]*$/
    MULTILINE_COMMENT: /\\/\\*[^*]*\\*+(?:[^/*][^*]*\\*+)*\\//

    comment: INLINE_COMMENT | MULTILINE_COMMENT

    import_statement: ^SPACES? "@" SPACES? "import" SPACES STRING_LIT SPACES?$

    supdef_block_start: ^SPACES? "@" SPACES? "supdef" SPACES "begin" SPACES IDENTIFIER
"""