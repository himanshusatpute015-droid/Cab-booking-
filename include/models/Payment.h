#ifndef CAB_MODELS_PAYMENT_H
#define CAB_MODELS_PAYMENT_H

#include <memory>
#include <string>
#include <vector>

namespace cab {

// INTERFACE (pure abstract class) for the STRATEGY pattern: the booking code works with
// any Payment without knowing which concrete method was chosen.
class Payment {
public:
    virtual ~Payment() {}

    virtual std::string method() const = 0;
    virtual std::string label() const = 0;
    virtual double processingFee(double amount) const = 0;
    virtual std::string pay(double amount, int bookingId) const = 0;  // returns a transaction id

protected:
    std::string makeTransactionId(const std::string& prefix, int bookingId) const;
};

class CashPayment : public Payment {
public:
    std::string method() const override { return "Cash"; }
    std::string label() const override { return "Cash - pay the driver"; }
    double processingFee(double) const override { return 0.0; }
    std::string pay(double amount, int bookingId) const override;
};

class UpiPayment : public Payment {
public:
    std::string method() const override { return "UPI"; }
    std::string label() const override { return "UPI - no extra fee"; }
    double processingFee(double) const override { return 0.0; }
    std::string pay(double amount, int bookingId) const override;
};

class CardPayment : public Payment {
public:
    static const double FEE_RATE;
    std::string method() const override { return "Card"; }
    std::string label() const override { return "Credit / Debit card - 2% fee"; }
    double processingFee(double amount) const override { return amount * FEE_RATE; }
    std::string pay(double amount, int bookingId) const override;
};

class WalletPayment : public Payment {
public:
    std::string method() const override { return "Wallet"; }
    std::string label() const override { return "CabGo Wallet - 1% fee"; }
    double processingFee(double amount) const override { return amount * 0.01; }
    std::string pay(double amount, int bookingId) const override;
};

// FACTORY: builds the correct Payment object from its name.
class PaymentFactory {
public:
    PaymentFactory() = delete;  // static-only utility class
    static std::unique_ptr<Payment> create(const std::string& method);
    static std::vector<std::string> methods();
};

}  // namespace cab

#endif
