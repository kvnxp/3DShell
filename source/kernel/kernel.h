#ifndef KERNEL_H
#define KERNEL_H

/* System core: platform services shared by the shell (network, logging,
 * debug channel). One init, module folders per concern. */
namespace Kernel {
    bool Init();
    void Poll();
    void Exit();
}

#endif // KERNEL_H
