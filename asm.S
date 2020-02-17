.intel_syntax noprefix

// uint8_t ReadIOPort8(uint16_t);
.global ReadIOPort8
ReadIOPort8:
  mov edx, ecx
  xor rax, rax
  in al, dx
  ret
