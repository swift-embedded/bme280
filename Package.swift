// swift-tools-version:5.0
import PackageDescription

let package = Package(
    name: "BME280",
    products: [
        .library(name: "BME280", targets: ["BME280"]),
    ],
    dependencies: [
        .package(url: "https://github.com/swift-embedded/hardware", .branch("master")),
    ],
    targets: [
        .target(name: "BME280", dependencies: ["Compensation", "Hardware"]),
        .target(name: "Compensation"),
    ]
)
