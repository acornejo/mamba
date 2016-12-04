# Macros

Implementing the ternary operator

    let cond = macro (c Expr, a Expr, b Expr) -> Expr {
        if c {
            return a
        } else {
            return b
        }
    }

    let a = 1, b = 2
    let x = cond!(a < b, a, b)

Implementing ruby-like string interpolation (uses StringExpr)

    let format = macro (fmt String) -> StringExpr {
        let &result = ""
        for token in tokenize(fmt) {
            if token.isExpression {
                result += "+ (" + token.str + ").toString()"
            } else {
                result += quote(token.str)
            }
        }
        return result
    }

    let name = "alex";
    let x = format!("hello \{name}, how are you?")

Unwrap or return (uses return!)

    let unwrapOrReturn = macro <T>(status StatusOr<T>) -> T {
        switch status {
            case Value(t) {
                return t
            }
            case Status(s) {
                return! s
            }
        }
    }

    let readCSV = fun (name String) -> StatusOr<CSV> {
        let f = unwrapOrReturn!(open(name))

        for line in f.readLines() {
            let &values = []
            for val in line.split(',') {
                values.push(val)
            }
            csv.push(values)
        }

        return csv
    }
