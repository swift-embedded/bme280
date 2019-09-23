import Compensation
import Hardware

public final class BME280Sensor {
    public let address: Int

    private let i2c: I2CMemory

    private static let MainCalibrationDataLength = 26
    private static let HumidityCalibrationDataLength = 7
    private static let MeasurementDataLength = 8

    enum Register: UInt8 {
        case measurementData = 0xF7
        case mainCalibrationData = 0x88
        case humidityCalibrationData = 0xE1
        case id = 0xD0
        case reset = 0xE0
        case status = 0xF3
        case powerControl = 0xF4
        case humidityControl = 0xF2
    }

    enum PowerMode: UInt8 {
        case sleep = 0x00
        case forced = 0x01
        case normal = 0x11
    }

    enum Oversampling: UInt8 {
        case skipped = 0x00
        case x01 = 0b001
        case x02 = 0b010
        case x04 = 0b011
        case x08 = 0b100
        case x16 = 0b101
    }

    public enum Error: Swift.Error {
        case invalidIdentifier
    }

    public struct Measurement {
        public let temperature: Float
        public let pressure: Float
        public let humidity: Float
    }

    public init(i2c: I2CMemory, address: Int = 0x76) {
        self.i2c = i2c
        self.address = address
    }

    public func reset() throws {
        try write(.reset, value: 0x00)
    }

    public func check() throws {
        if try read(.id) != 0x60 {
            throw Error.invalidIdentifier
        }
    }

    public func measure() throws -> Measurement {
        var calibrationData = try readCalibrationData()

        try setOversampling()
        try setPowerMode(.forced)

        while (try read(.status) & 0b1001) != 0 {}

        let raw = try i2c.read(address: address,
                               register: Register.measurementData.rawValue,
                               length: Self.MeasurementDataLength)
        var uncompensated: bme280_uncomp_data = raw.withUnsafeBufferPointer { buffer in
            var data = bme280_uncomp_data()
            bme280_parse_sensor_data(buffer.baseAddress, &data)
            return data
        }
        var compensated = bme280_data()
        bme280_compensate_data(0b111, &uncompensated, &compensated, &calibrationData)
        return Measurement(temperature: Float(compensated.temperature) / 100,
                           pressure: Float(compensated.pressure) / 100,
                           humidity: Float(compensated.humidity) / 1024)
    }

    private func read(_ register: Register) throws -> UInt8 {
        return try i2c.read(address: address, register: register.rawValue, length: 1)[0]
    }

    private func write(_ register: Register, value: UInt8) throws {
        try i2c.write(address: address, register: register.rawValue, data: [value])
    }

    private func setPowerMode(_ mode: PowerMode) throws {
        let state = try read(.powerControl) & ~0b11
        try write(.powerControl, value: state | mode.rawValue)
    }

    private func setOversampling(temperature: Oversampling = .x01,
                                 humidity: Oversampling = .x01,
                                 pressure: Oversampling = .x01) throws {
        try write(.humidityControl, value: humidity.rawValue)

        var state = try read(.powerControl) & 0x03
        state |= temperature.rawValue << 5
        state |= pressure.rawValue << 2
        try write(.powerControl, value: state)
    }

    private func readCalibrationData() throws -> bme280_calib_data {
        var data = bme280_calib_data()

        // read main calibration data
        var raw = try i2c.read(address: address,
                               register: Register.mainCalibrationData.rawValue,
                               length: Self.MainCalibrationDataLength)
        raw.withUnsafeBufferPointer { parse_temp_press_calib_data($0.baseAddress, &data) }

        // read humidity calibration data
        raw = try i2c.read(address: address,
                           register: Register.humidityCalibrationData.rawValue,
                           length: Self.HumidityCalibrationDataLength)
        raw.withUnsafeBufferPointer { parse_humidity_calib_data($0.baseAddress, &data) }

        return data
    }
}
