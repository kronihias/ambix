; Shared include for all ambix installer scripts.
; Widens the MUI2 installer window by INSTALLER_EXTRA_WIDTH pixels so that
; longer text (component names, install paths, etc.) is readable.

!include LogicLib.nsh

!ifndef INSTALLER_EXTRA_WIDTH
    !define INSTALLER_EXTRA_WIDTH 150
!endif

; SWP_NOMOVE | SWP_NOZORDER
!define SWP_SIZE_ONLY 0x0006

Function .onGUIInit
    ; --- Resize the outer installer window ---
    System::Call 'user32::GetWindowRect(p $HWNDPARENT, @r0)'
    System::Call '*$0(i.r1, i.r2, i.r3, i.r4)'
    IntOp $5 $3 - $1
    IntOp $6 $4 - $2
    IntOp $5 $5 + ${INSTALLER_EXTRA_WIDTH}
    System::Call 'user32::SetWindowPos(p $HWNDPARENT, p 0, i 0, i 0, i $5, i $6, i ${SWP_SIZE_ONLY})'

    ; --- Resize the inner page container so page content fills the extra width ---
    FindWindow $7 "#32770" "" $HWNDPARENT
    ${If} $7 <> 0
        System::Call 'user32::GetWindowRect(p $7, @r0)'
        System::Call '*$0(i.r1, i.r2, i.r3, i.r4)'
        IntOp $8 $3 - $1
        IntOp $9 $4 - $2
        IntOp $8 $8 + ${INSTALLER_EXTRA_WIDTH}
        System::Call 'user32::SetWindowPos(p $7, p 0, i 0, i 0, i $8, i $9, i ${SWP_SIZE_ONLY})'
    ${EndIf}
FunctionEnd
