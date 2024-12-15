#include "MyForm.h"

using namespace System;  // Managed code namespace
using namespace System::Windows::Forms;

[STAThreadAttribute]
void main(cli::array<String^>^ args) {  // Fully qualified name for managed array
    // Enable visual styles and set text rendering
    Application::SetCompatibleTextRenderingDefault(false);
    Application::EnableVisualStyles();

    // Create and run the form
    GraphPlotter::GraphForm frm;
    Application::Run(% frm);
}
