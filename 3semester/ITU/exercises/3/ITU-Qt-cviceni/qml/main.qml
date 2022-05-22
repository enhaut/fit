import QtQuick 2.1
import QtQuick.Window 2.0
import "../js/Theme.js" as Theme
import cz.vutbr.fit 1.0

Window {
    visible: true
    width: 400
    height: 315
    
    title: "ITU - Qt 5 / QML kalkulačka"

    // Definování datového modelu s operátory
    // 'op' - zkratka pro operaci
    // 'tog' - zkratka pro toggled, označení, která operace je vybrána
    ListModel {
        id: operations;
        ListElement { op: "+"; tog: true; }
        ListElement { op: "-"; tog: false; }
        ListElement { op: "*"; tog: false; }
        ListElement { op: "/"; tog: false; }
    }

    // Prvek pro rozvržení prvků do sloupce
    // http://en.wikipedia.org/wiki/Layout_%28computing%29
    // https://qmlbook.github.io/ch04-qmlstart/qmlstart.html#positioning-elements
    Column {

        // Vstupní hodnota - první operand výpočtu
        Rectangle {
            id: input_row
            height: 35;
            width: 400;
            border.color: "#bbb";
            border.width: 3;
            anchors.margins: 2
            color: "#777"


            TextInput {
                anchors.fill: parent;
                anchors.margins: 2
                horizontalAlignment: TextInput.AlignLeft
                verticalAlignment: TextInput.AlignVCenter
                id: textA
                font.pointSize: 22
                text: "0"
                
            }
        }

        // Prvek pro rozvržení prvků do řádku
        // Více jak prvek Column (výše)
        Row {
            // Obdoba ListView (ale více obecný) nebo funkce foreach()
            // obsahuje _model_ a _delegate_
            Repeater {
                // Definování modelu, data pro zobrazení
                model: operations;

                // Delegování vzhledu v MVC
                // Jak má vypadat jeden prvek
                delegate: MyButton {
                    btnColor: Theme.btn_colour
                    
                    text: model.op
                    toggled: model.tog;
                    
                    onClicked: {
                        for (var i = 0; i < operations.count; i++) {
                            operations.setProperty( i, "tog", (i == index) );
                        }
                    }
                }
            }

        }

        // "Vlastní" třída pro posuvník. Definice v MySlider.qml
        MySlider {
            id: slider
            color: Qt.darker(Theme.slider_color)
            rectColor: Theme.slider_color
        }

        // TODO
        // vložte nový textový prvek, který bude bude vizuálně 'zapadat'
        // do výsledné aplikace a bude zobrazoval vertikálně vycentrovaný
        // text 'LUT value: ' a hodnotu aktuálně vybrané položky z LUT
        Rectangle {
            height: 35;
            width: 400;
            border.color: "#bbb";
            color: "#777"
            id: lut_area

            Text {
                id:  lut_value;
                height: 35;
                font.pointSize: 17
                color: "#000"
                text: "LUT value: " + lut.getValue(slider.value)
            }
        }


        // Vlastní klikací tlačítko. Definice v MyClickButton.qml
        MyClickButton {
            width: 400;
            btnColor: Theme.btn_colour
            
            text: qsTr( "Compute" )
            
            function getOperation() {
                for (var i = 0; i < operations.count; i++) {
                    var item = operations.get(i);
                    if (item.tog) {
                        return item.op;
                    }
                }
                return "+";
            }

            // Obsluha události při zachycení signálu clicked
            onClicked: {
                var a = parseFloat(textA.text, 10);
                var b = lut.getValue(slider.value);
                var op = getOperation();
                var ok = 1;

                var red = "#FF0000"
                if (isNaN(a))
                {
                    input_row.color = red;
                    result.text = qsTr("Invalid input");
                    result.color = red;
                    lut_area.color = "#777";
                    slider.rectColor = "#4682b4";
                    ok = 0;
                }else if(b === 0 && op === "/") {
                    input_row.color = "#777";
                    result.text = qsTr("Zero division error");
                    result.color = red;
                    slider.rectColor = red;
                    lut_area.color = red;
                    ok = 0;
                }else{
                    input_row.color = "#777";
                    result.color = "#0066FF";
                    slider.rectColor = "#4682b4";
                    lut_area.color = "#777";
                    ok = 1;
                }

                console.log( a + " "+ op + " " + b + " = ?")
                
                if (ok === 1)
                {
                    var res = 0.0;
                    if (op === "+")
                        res = a + b;
                    else if (op === "-")
                        res = a - b;
                    else if (op === "/")
                        res = a / b;
                    else if (op === "*")
                        res = a * b;
                    result.text = res;
                }
            }
        }

        // Prvek pro zobrazení výsledné hodnoty
        Rectangle {
            height: 45;
            width: 400;
            border.color: "#bbb";
            border.width: 3;
            anchors.margins: 2
            color: "#777"
            
            Text {
                id:  result;
                anchors.centerIn: parent
                height: 35;
                font.pointSize: 22
                color: "#0066FF"
            }
        }

    }

    // Vytvoření objektu LUT, který je definován v C++
    // K danému se přistupuje pomocí jeho id
    LUT {
        id: lut
    }

}

