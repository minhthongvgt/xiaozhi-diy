#ifndef DUAL_DISPLAY_H
#define DUAL_DISPLAY_H

#include "display.h"
#include <esp_log.h>

/**
 * @brief Dual Display Proxy Class for Xiaozhi
 * Allows running 2 displays in parallel (e.g. Primary SPI LCD / OLED + Secondary External UART Display).
 * All display status updates, chat messages, notifications, and animations are forwarded to both displays.
 */
class DualDisplay : public Display {
private:
    Display* primary_ = nullptr;
    Display* secondary_ = nullptr;

public:
    DualDisplay(Display* primary, Display* secondary)
        : primary_(primary), secondary_(secondary) {
        if (primary_) {
            width_ = primary_->width();
            height_ = primary_->height();
        } else if (secondary_) {
            width_ = secondary_->width();
            height_ = secondary_->height();
        }
    }

    virtual ~DualDisplay() {
        if (primary_) {
            delete primary_;
            primary_ = nullptr;
        }
        if (secondary_) {
            delete secondary_;
            secondary_ = nullptr;
        }
    }

    virtual void SetStatus(const char* status) override {
        if (primary_) primary_->SetStatus(status);
        if (secondary_) secondary_->SetStatus(status);
    }

    virtual void ShowNotification(const char* notification, int duration_ms = 3000) override {
        if (primary_) primary_->ShowNotification(notification, duration_ms);
        if (secondary_) secondary_->ShowNotification(notification, duration_ms);
    }

    virtual void ShowNotification(const std::string& notification, int duration_ms = 3000) override {
        if (primary_) primary_->ShowNotification(notification, duration_ms);
        if (secondary_) secondary_->ShowNotification(notification, duration_ms);
    }

    virtual void SetEmotion(const char* emotion) override {
        if (primary_) primary_->SetEmotion(emotion);
        if (secondary_) secondary_->SetEmotion(emotion);
    }

    virtual void SetChatMessage(const char* role, const char* content) override {
        if (primary_) primary_->SetChatMessage(role, content);
        if (secondary_) secondary_->SetChatMessage(role, content);
    }

    virtual void ClearChatMessages() override {
        if (primary_) primary_->ClearChatMessages();
        if (secondary_) secondary_->ClearChatMessages();
    }

    virtual void SetTheme(Theme* theme) override {
        if (primary_) primary_->SetTheme(theme);
        if (secondary_) secondary_->SetTheme(theme);
    }

    virtual void UpdateStatusBar(bool update_all = false) override {
        if (primary_) primary_->UpdateStatusBar(update_all);
        if (secondary_) secondary_->UpdateStatusBar(update_all);
    }

    virtual void SetPowerSaveMode(bool on) override {
        if (primary_) primary_->SetPowerSaveMode(on);
        if (secondary_) secondary_->SetPowerSaveMode(on);
    }

    virtual void SetupUI() override {
        if (primary_) primary_->SetupUI();
        if (secondary_) secondary_->SetupUI();
        setup_ui_called_ = true;
    }

    virtual bool IsMonochrome() const override {
        return primary_ ? primary_->IsMonochrome() : false;
    }

    virtual bool SupportsGuiOperations() const override {
        return primary_ ? primary_->SupportsGuiOperations() : false;
    }

    Display* GetPrimary() const { return primary_; }
    Display* GetSecondary() const { return secondary_; }

protected:
    virtual bool Lock(int timeout_ms = 0) override {
        bool p_ok = primary_ ? primary_->Lock(timeout_ms) : true;
        bool s_ok = secondary_ ? secondary_->Lock(timeout_ms) : true;
        return p_ok && s_ok;
    }

    virtual void Unlock() override {
        if (primary_) primary_->Unlock();
        if (secondary_) secondary_->Unlock();
    }
};

#endif // DUAL_DISPLAY_H
